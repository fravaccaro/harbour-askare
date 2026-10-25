#include "taskmodel.h"

#include "ics.h"
#include "store.h"

TaskModel::TaskModel(Store *store, QObject *parent)
    : QAbstractListModel(parent)
    , m_store(store)
{
}

void TaskModel::setCalendarHref(const QString &href)
{
    if (href == m_calendarHref)
        return;

    m_calendarHref = href;
    emit calendarHrefChanged();
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
    m_items = m_store->tasks(m_calendarHref);
    endResetModel();
}
