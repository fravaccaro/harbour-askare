#include "taskmodel.h"

#include <algorithm>

#include <QDate>

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

QVector<LocalTask> todayOf(const QVector<LocalTask> &tasks)
{
    QVector<LocalTask> out;
    for (const LocalTask &task : tasks) {
        if (Ics::isToday(task.status, task.due))
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

// Whether a task carries the query in any text a reader would scan for it:
// its title, its notes, where it is, and the tags it wears.
bool taskMatches(const LocalTask &task, const QString &needle)
{
    if (task.summary.contains(needle, Qt::CaseInsensitive))
        return true;
    if (task.description.contains(needle, Qt::CaseInsensitive))
        return true;
    if (task.location.contains(needle, Qt::CaseInsensitive))
        return true;
    for (const QString &tag : task.tags) {
        if (tag.contains(needle, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

// The tasks that answer the query, in the order they arrived.
QVector<LocalTask> matchingOf(const QVector<LocalTask> &tasks, const QString &needle)
{
    QVector<LocalTask> out;
    for (const LocalTask &task : tasks) {
        if (taskMatches(task, needle))
            out.append(task);
    }
    return out;
}

// Whether a list of chosen values names this token, ignoring case, so a filter
// matches whichever of its choices a task carries.
bool chosenHas(const QVariantList &values, const QString &token)
{
    for (const QVariant &value : values) {
        if (value.toString().compare(token, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

// The same for a calendar, compared on the slash-normalised path the colour
// bar already uses, so the href a task stores and the one a filter holds meet.
bool chosenHasHref(const QVariantList &values, const QString &href)
{
    const QString wanted = Url::pathWithSlash(href);
    for (const QVariant &value : values) {
        if (Url::pathWithSlash(value.toString()) == wanted)
            return true;
    }
    return false;
}

bool chosenHasTag(const QStringList &tags, const QVariantList &values)
{
    for (const QVariant &value : values) {
        if (tags.contains(value.toString(), Qt::CaseInsensitive))
            return true;
    }
    return false;
}

// The priority band a value falls in, the four the editor shows.
QString priorityBand(int priority)
{
    if (priority >= 1 && priority <= 4)
        return QStringLiteral("high");
    if (priority == 5)
        return QStringLiteral("medium");
    if (priority >= 6 && priority <= 9)
        return QStringLiteral("low");
    return QStringLiteral("none");
}

// A task's completion the way the row reads it: a completed task is whole.
int completionOf(const LocalTask &task)
{
    return Ics::percentFromStatus(task.status, Ics::parse(task.ics).percent);
}

// Whether a task's day answers a date filter. A task with no such day fails:
// an empty day can neither fall on, before nor after the one that was chosen.
bool dayMatches(const QString &day, const QString &chosen, const QString &mode)
{
    if (day.isEmpty())
        return false;
    if (mode == QLatin1String("before"))
        return day < chosen;
    if (mode == QLatin1String("after"))
        return day > chosen;
    return day == chosen;
}

QString today()
{
    return QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
}

bool reminderWanted(const QVariantMap &f)
{
    const QString want = f.value(QStringLiteral("reminder")).toString();
    return want == QLatin1String("yes") || want == QLatin1String("no");
}

// A saved search keeps its query inside the filter set, under a key of its own
// that no filter reads.
QString searchOf(const QVariantMap &f)
{
    return f.value(QStringLiteral("search")).toString();
}

// How many of the form's rows are set. One being set is what makes the filters
// active at all, so the two answers come from this one list.
int activeFilterCount(const QVariantMap &f)
{
    const bool set[] = {
        !f.value(QStringLiteral("status")).toList().isEmpty(),
        !f.value(QStringLiteral("lists")).toList().isEmpty(),
        !f.value(QStringLiteral("priority")).toList().isEmpty(),
        !f.value(QStringLiteral("tags")).toList().isEmpty(),
        f.value(QStringLiteral("completion")).toInt() > 0,
        f.value(QStringLiteral("dueToday")).toBool(),
        !f.value(QStringLiteral("startDate")).toString().isEmpty(),
        !f.value(QStringLiteral("dueDate")).toString().isEmpty(),
        reminderWanted(f)
    };
    return std::count(set, set + (sizeof(set) / sizeof(set[0])), true);
}

bool statusPasses(const LocalTask &task, const QVariantMap &f)
{
    const QVariantList chosen = f.value(QStringLiteral("status")).toList();
    return chosen.isEmpty() || chosenHas(chosen, task.status);
}

bool listPasses(const LocalTask &task, const QVariantMap &f)
{
    const QVariantList chosen = f.value(QStringLiteral("lists")).toList();
    return chosen.isEmpty() || chosenHasHref(chosen, task.calendarHref);
}

bool priorityPasses(const LocalTask &task, const QVariantMap &f)
{
    const QVariantList chosen = f.value(QStringLiteral("priority")).toList();
    return chosen.isEmpty() || chosenHas(chosen, priorityBand(task.priority));
}

bool tagsPass(const LocalTask &task, const QVariantMap &f)
{
    const QVariantList chosen = f.value(QStringLiteral("tags")).toList();
    return chosen.isEmpty() || chosenHasTag(task.tags, chosen);
}

bool completionPasses(const LocalTask &task, const QVariantMap &f)
{
    const int threshold = f.value(QStringLiteral("completion")).toInt();
    if (threshold <= 0)
        return true;

    const int percent = completionOf(task);
    if (f.value(QStringLiteral("completionMode")).toString() == QLatin1String("atMost"))
        return percent <= threshold;
    return percent >= threshold;
}

bool dueTodayPasses(const LocalTask &task, const QVariantMap &f)
{
    if (!f.value(QStringLiteral("dueToday")).toBool())
        return true;
    return Ics::dateFromDue(task.due) == today();
}

bool startPasses(const LocalTask &task, const QVariantMap &f)
{
    const QString chosen = f.value(QStringLiteral("startDate")).toString();
    if (chosen.isEmpty())
        return true;
    const QString day = Ics::dateFromDue(Ics::parse(task.ics).start);
    return dayMatches(day, chosen, f.value(QStringLiteral("startMode")).toString());
}

bool duePasses(const LocalTask &task, const QVariantMap &f)
{
    const QString chosen = f.value(QStringLiteral("dueDate")).toString();
    if (chosen.isEmpty())
        return true;
    return dayMatches(Ics::dateFromDue(task.due), chosen,
                      f.value(QStringLiteral("dueMode")).toString());
}

bool reminderPasses(const LocalTask &task, const QVariantMap &f)
{
    const QString want = f.value(QStringLiteral("reminder")).toString();
    if (want == QLatin1String("yes"))
        return task.reminder >= 0;
    if (want == QLatin1String("no"))
        return task.reminder < 0;
    return true;
}

// A task is kept only when every filter that is set accepts it.
bool passesFilters(const LocalTask &task, const QVariantMap &f)
{
    return statusPasses(task, f) && listPasses(task, f) && priorityPasses(task, f)
            && tagsPass(task, f) && completionPasses(task, f) && dueTodayPasses(task, f)
            && startPasses(task, f) && duePasses(task, f) && reminderPasses(task, f);
}

QVector<LocalTask> filteredBy(const QVector<LocalTask> &tasks, const QVariantMap &f)
{
    QVector<LocalTask> out;
    for (const LocalTask &task : tasks) {
        if (passesFilters(task, f))
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
    , m_smartListId(-1)
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
    if (m_filter == QLatin1String("today"))
        return tr("Today");
    if (m_filter == QLatin1String("important"))
        return tr("Important");
    if (m_filter == QLatin1String("completed"))
        return tr("Completed");
    if (m_filter == QLatin1String("calendar"))
        return m_calendarName;
    if (m_filter == QLatin1String("tag"))
        return m_tag;
    // A smart view carries its own name, as a calendar does.
    if (m_filter == QLatin1String("smart"))
        return m_calendarName;
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

void TaskModel::showToday()
{
    apply(QStringLiteral("today"), QString(), QString());
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

void TaskModel::showSmart(int id, const QString &name, const QVariantMap &filters)
{
    if (id != m_smartListId) {
        m_smartListId = id;
        emit smartListIdChanged();
    }
    m_viewFilters = filters;
    // The saved search carries its own query. One left over from the live
    // search would otherwise narrow it further.
    if (!m_search.isEmpty()) {
        m_search.clear();
        emit searchChanged();
    }
    if (!m_filters.isEmpty()) {
        m_filters.clear();
        emit filtersChanged();
    }
    apply(QStringLiteral("smart"), QString(), name);
}

void TaskModel::forgetSmartList()
{
    m_viewFilters.clear();
    if (m_smartListId < 0)
        return;

    m_smartListId = -1;
    emit smartListIdChanged();
}

int TaskModel::countMatching(const QVariantMap &filters) const
{
    return matchedBy(searchOf(filters), filters).size();
}

QVariantMap TaskModel::searchSet() const
{
    QVariantMap out = m_filters;
    // Left out when nothing is typed, so a set of filters alone is saved as
    // plainly as it reads.
    if (!m_search.isEmpty())
        out.insert(QStringLiteral("search"), m_search);
    return out;
}

void TaskModel::setSearch(const QString &text)
{
    const QString next = text.trimmed();
    if (next == m_search)
        return;

    m_search = next;
    emit searchChanged();
    reload();
}

void TaskModel::setFilters(const QVariantMap &filters)
{
    if (filters == m_filters)
        return;

    m_filters = filters;
    emit filtersChanged();
    reload();
}

int TaskModel::filterCount() const
{
    return activeFilterCount(m_filters);
}

bool TaskModel::filtersActive() const
{
    return filterCount() > 0;
}

void TaskModel::apply(const QString &filter, const QString &href, const QString &name)
{
    // Every other view leaves the smart one behind; showSmart() sets its own
    // state before calling here, so only the others clear it.
    if (filter != QLatin1String("smart"))
        forgetSmartList();

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

QVector<LocalTask> TaskModel::withSettings(const QVector<LocalTask> &tasks) const
{
    QVector<LocalTask> out = tasks;
    if (!m_showCompleted)
        out = openOf(out);
    if (!m_showCancelled)
        out = withoutCancelled(out);
    return out;
}

// Current, Today and Completed stay flat: a subtask due today must not hide
// behind its parent in the views that answer "what now" and "what is done".
QVector<LocalTask> TaskModel::filtered() const
{
    if (m_filter == QLatin1String("current"))
        return currentOf(m_store->tasks());
    if (m_filter == QLatin1String("today"))
        return todayOf(m_store->tasks());
    if (m_filter == QLatin1String("important"))
        return importantOf(m_store->tasks());
    if (m_filter == QLatin1String("completed"))
        return completedOf(m_store->tasks());
    if (m_filter == QLatin1String("tag"))
        return taggedOf(m_store->tasks(), m_tag);
    if (m_filter == QLatin1String("calendar"))
        return m_store->topLevelTasks(m_calendarHref);
    return m_store->topLevelTasks();
}

// Every list, at every depth, finished and cancelled included. The view the
// search was opened from plays no part, or a matching subtask would stay
// hidden behind a parent, a due date, or a list the user was not looking at.
// The query and the filters each narrow the set and are applied together, and
// either of them may be empty.
QVector<LocalTask> TaskModel::matchedBy(const QString &query, const QVariantMap &filters) const
{
    QVector<LocalTask> out = m_store->tasks();
    if (!query.isEmpty())
        out = matchingOf(out, query);
    return filteredBy(out, filters);
}

QVector<LocalTask> TaskModel::searched() const
{
    return matchedBy(m_search, m_filters);
}

// A smart view is a search that was saved, so it is read the same way: the
// query it kept and the filters it kept, together.
QVector<LocalTask> TaskModel::smartFiltered() const
{
    return matchedBy(searchOf(m_viewFilters), m_viewFilters);
}

void TaskModel::reload()
{
    beginResetModel();
    // A query or a filter searches every list; otherwise the view holds, and
    // only the mixed views answer the completed and cancelled settings.
    if (!m_search.isEmpty() || filtersActive()) {
        m_items = searched();
    } else if (m_filter == QLatin1String("smart")) {
        m_items = smartFiltered();
    } else {
        m_items = filtered();
        if (isMixedView())
            m_items = withSettings(m_items);
    }
    m_tallies = talliesOf(m_store->tasks());
    m_colors = colorsOf(m_store->calendars());
    endResetModel();
}
