#include "taskmodel.h"

#include "ics.h"
#include "store.h"

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

}

TaskModel::TaskModel(Store *store, QObject *parent)
    : QAbstractListModel(parent)
    , m_store(store)
    , m_filter(QStringLiteral("all"))
{
}

QString TaskModel::title() const
{
    if (m_filter == QLatin1String("current"))
        return tr("Current");
    if (m_filter == QLatin1String("calendar"))
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

void TaskModel::showCalendar(const QString &href, const QString &name)
{
    apply(QStringLiteral("calendar"), href, name);
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
    roles.insert(UidRole, "uid");
    roles.insert(SummaryRole, "summary");
    roles.insert(DescriptionRole, "description");
    roles.insert(StatusRole, "status");
    roles.insert(DueRole, "due");
    roles.insert(DueDateRole, "dueDate");
    roles.insert(PriorityRole, "priority");
    roles.insert(PercentRole, "percent");
    roles.insert(CompletedRole, "completed");
    roles.insert(DoneRole, "done");
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
    case PriorityRole:
        return task.priority;
    case PercentRole:
        return Ics::percentFromStatus(task.status, Ics::parse(task.ics).percent);
    case CompletedRole:
        return task.completed;
    case DoneRole:
        return Ics::isCompleted(task.status);
    default:
        return QVariant();
    }
}

void TaskModel::reload()
{
    beginResetModel();
    if (m_filter == QLatin1String("current"))
        m_items = currentOf(m_store->tasks());
    else if (m_filter == QLatin1String("calendar"))
        m_items = m_store->tasks(m_calendarHref);
    else
        m_items = m_store->tasks();
    endResetModel();
}
