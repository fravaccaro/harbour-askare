#include "taskmodel.h"

#include "ics.h"
#include "store.h"
#include "url.h"

namespace {

QVector<LocalTask> currentOf(const QVector<LocalTask> &tasks)
{
    QVector<LocalTask> out;
    for (const LocalTask &task : tasks) {
        if (Ics::isCurrent(task.status, task.due))
            out.append(task);
    }
    return out;
}

// Drops only completed tasks, so an open task due later still shows. This is
// what currentOf() does not do, since that view also hides what is not due yet.
QVector<LocalTask> openOf(const QVector<LocalTask> &tasks)
{
    QVector<LocalTask> out;
    for (const LocalTask &task : tasks) {
        if (!Ics::isCompleted(task.status))
            out.append(task);
    }
    return out;
}

// Drops cancelled tasks, which answer the setting of their own.
QVector<LocalTask> withoutCancelled(const QVector<LocalTask> &tasks)
{
    QVector<LocalTask> out;
    for (const LocalTask &task : tasks) {
        if (!Ics::isCancelled(task.status))
            out.append(task);
    }
    return out;
}

// The section a row belongs to, so the list draws one header per group.
QString groupOf(const QString &status)
{
    if (Ics::isCancelled(status))
        return QStringLiteral("cancelled");
    if (Ics::isCompleted(status))
        return QStringLiteral("completed");
    return QStringLiteral("todo");
}

// The Completed view gathers everything no longer open: the tasks that were
// finished and the ones that were cancelled, each under its own section.
QVector<LocalTask> completedOf(const QVector<LocalTask> &tasks)
{
    QVector<LocalTask> out;
    for (const LocalTask &task : tasks) {
        if (Ics::isCompleted(task.status) || Ics::isCancelled(task.status))
            out.append(task);
    }
    return out;
}

// The high-priority band, flat like the other smart lists so a key subtask
// surfaces on its own.
QVector<LocalTask> importantOf(const QVector<LocalTask> &tasks)
{
    QVector<LocalTask> out;
    for (const LocalTask &task : tasks) {
        if (Ics::isImportant(task.status, task.priority))
            out.append(task);
    }
    return out;
}

// Every task carrying one tag, whatever its place in the tree, so a tagged
// subtask surfaces beside the others.
QVector<LocalTask> taggedOf(const QVector<LocalTask> &tasks, const QString &tag)
{
    QVector<LocalTask> out;
    for (const LocalTask &task : tasks) {
        if (task.tags.contains(tag, Qt::CaseInsensitive))
            out.append(task);
    }
    return out;
}

// A row shows which list owns it by colour, so the colours are read once per
// reload instead of once per row.
QHash<QString, QString> colorsOf(const QVector<CalendarInfo> &calendars)
{
    QHash<QString, QString> out;
    for (const CalendarInfo &info : calendars)
        out.insert(Url::pathWithSlash(info.href), info.color);
    return out;
}

// One pass over every task, so a row's subtask counts cost no extra query.
QHash<QString, QPair<int, int> > talliesOf(const QVector<LocalTask> &tasks)
{
    QHash<QString, QPair<int, int> > out;
    for (const LocalTask &task : tasks) {
        if (task.parentUid.isEmpty())
            continue;
        QPair<int, int> &tally = out[task.parentUid];
        ++tally.first;
        if (Ics::isCompleted(task.status))
            ++tally.second;
    }
    return out;
}

}

TaskModel::TaskModel(Store *store, QObject *parent)
    : QAbstractListModel(parent)
    , m_store(store)
    , m_filter(QStringLiteral("all"))
    , m_showCompleted(true)
    , m_showCancelled(false)
{
}

// The views that mix open and completed rows, and so answer to the setting.
bool TaskModel::isMixedView() const
{
    return m_filter == QLatin1String("all") || m_filter == QLatin1String("calendar")
            || m_filter == QLatin1String("tag");
}

void TaskModel::setShowCompleted(bool show)
{
    if (show == m_showCompleted)
        return;

    m_showCompleted = show;
    reload();
}

void TaskModel::setShowCancelled(bool show)
{
    if (show == m_showCancelled)
        return;

    m_showCancelled = show;
    reload();
}

QString TaskModel::title() const
{
    if (m_filter == QLatin1String("current"))
        return tr("Current");
    if (m_filter == QLatin1String("important"))
        return tr("Important");
    if (m_filter == QLatin1String("completed"))
        return tr("Completed");
    if (m_filter == QLatin1String("calendar"))
        return m_calendarName;
    if (m_filter == QLatin1String("tag"))
        return m_tag;
    return tr("All");
}

void TaskModel::showAll()
{
    apply(QStringLiteral("all"), QString(), QString());
}

void TaskModel::showCurrent()
{
    apply(QStringLiteral("current"), QString(), QString());
}

void TaskModel::showImportant()
{
    apply(QStringLiteral("important"), QString(), QString());
}

void TaskModel::showCompleted()
{
    apply(QStringLiteral("completed"), QString(), QString());
}

void TaskModel::showCalendar(const QString &href, const QString &name)
{
    apply(QStringLiteral("calendar"), href, name);
}

void TaskModel::showTag(const QString &tag)
{
    if (tag != m_tag) {
        m_tag = tag;
        emit tagChanged();
    }
    // The tag also travels as the view's name, so apply() notices a move from
    // one tag to another and the title follows.
    apply(QStringLiteral("tag"), QString(), tag);
}

void TaskModel::apply(const QString &filter, const QString &href, const QString &name)
{
    const bool same = filter == m_filter && href == m_calendarHref && name == m_calendarName;
    m_filter = filter;
    m_calendarHref = href;
    m_calendarName = name;
    if (!same) {
        emit filterChanged();
        emit calendarHrefChanged();
        emit titleChanged();
    }
    reload();
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QHash<int, QByteArray> TaskModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(HrefRole, "href");
    roles.insert(CalendarHrefRole, "calendarHref");
    roles.insert(CalendarColorRole, "calendarColor");
    roles.insert(UidRole, "uid");
    roles.insert(SummaryRole, "summary");
    roles.insert(DescriptionRole, "description");
    roles.insert(StatusRole, "status");
    roles.insert(DueRole, "due");
    roles.insert(DueDateRole, "dueDate");
    roles.insert(DueTimeRole, "dueTime");
    roles.insert(ReminderRole, "reminder");
    roles.insert(PriorityRole, "priority");
    roles.insert(PercentRole, "percent");
    roles.insert(CompletedRole, "completed");
    roles.insert(DoneRole, "done");
    roles.insert(GroupRole, "group");
    roles.insert(ChildCountRole, "childCount");
    roles.insert(ChildDoneCountRole, "childDoneCount");
    return roles;
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return QVariant();

    const LocalTask &task = m_items.at(index.row());
    switch (role) {
    case HrefRole:
        return task.href;
    case CalendarHrefRole:
        return task.calendarHref;
    case CalendarColorRole:
        return m_colors.value(Url::pathWithSlash(task.calendarHref));
    case UidRole:
        return task.uid;
    case SummaryRole:
        return task.summary;
    case DescriptionRole:
        return task.description;
    case StatusRole:
        return task.status;
    case DueRole:
        return task.due;
    case DueDateRole:
        return Ics::dateFromDue(task.due);
    case DueTimeRole:
        return Ics::timeFromDue(task.due);
    case ReminderRole:
        return task.reminder;
    case PriorityRole:
        return task.priority;
    case PercentRole:
        return Ics::percentFromStatus(task.status, Ics::parse(task.ics).percent);
    case CompletedRole:
        return task.completed;
    case DoneRole:
        return Ics::isCompleted(task.status);
    case GroupRole:
        return groupOf(task.status);
    case ChildCountRole:
        return tallyOf(task.uid).first;
    case ChildDoneCountRole:
        return tallyOf(task.uid).second;
    default:
        return QVariant();
    }
}

TaskModel::ChildTally TaskModel::tallyOf(const QString &uid) const
{
    return m_tallies.value(uid, ChildTally(0, 0));
}

void TaskModel::reload()
{
    beginResetModel();
    // Current and completed stay flat: a subtask due today must not hide
    // behind its parent in the views that answer "what now" and "what is done".
    if (m_filter == QLatin1String("current"))
        m_items = currentOf(m_store->tasks());
    else if (m_filter == QLatin1String("important"))
        m_items = importantOf(m_store->tasks());
    else if (m_filter == QLatin1String("completed"))
        m_items = completedOf(m_store->tasks());
    else if (m_filter == QLatin1String("tag"))
        m_items = taggedOf(m_store->tasks(), m_tag);
    else if (m_filter == QLatin1String("calendar"))
        m_items = m_store->topLevelTasks(m_calendarHref);
    else
        m_items = m_store->topLevelTasks();
    // Only the mixed views answer the two settings; the actionable views and
    // the completed one already decide membership by status.
    if (isMixedView()) {
        if (!m_showCompleted)
            m_items = openOf(m_items);
        if (!m_showCancelled)
            m_items = withoutCancelled(m_items);
    }
    m_tallies = talliesOf(m_store->tasks());
    m_colors = colorsOf(m_store->calendars());
    endResetModel();
}
