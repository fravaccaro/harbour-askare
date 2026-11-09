#include "smartlistmodel.h"

#include "config.h"
#include "store.h"
#include "taskmodel.h"

namespace {

const char *IdKey = "id";
const char *NameKey = "name";
const char *CountKey = "count";
const char *FiltersKey = "filters";

int idOf(const QVariantMap &entry)
{
    return entry.value(QLatin1String(IdKey)).toInt();
}

}

SmartListModel::SmartListModel(Config *config, TaskModel *tasks, Store *store,
                               QObject *parent)
    : QAbstractListModel(parent)
    , m_config(config)
    , m_tasks(tasks)
    , m_store(store)
{
}

int SmartListModel::count() const
{
    return m_items.size();
}

int SmartListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QHash<int, QByteArray> SmartListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(IdRole, IdKey);
    roles.insert(NameRole, NameKey);
    roles.insert(CountRole, CountKey);
    roles.insert(FiltersRole, FiltersKey);
    return roles;
}

QVariant SmartListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return QVariant();

    const QVariantMap &entry = m_items.at(index.row());
    switch (role) {
    case IdRole:
        return entry.value(QLatin1String(IdKey));
    case NameRole:
        return entry.value(QLatin1String(NameKey));
    case CountRole:
        return entry.value(QLatin1String(CountKey));
    case FiltersRole:
        return entry.value(QLatin1String(FiltersKey));
    default:
        return QVariant();
    }
}

// The saved sets as they stand, each with the number of tasks its filters
// keep. A store not yet open answers nothing, so the rows still appear with a
// count of zero rather than waiting for it.
QVector<QVariantMap> SmartListModel::computeItems() const
{
    const QVariantList saved = m_config->smartLists();
    QVector<QVariantMap> items;
    items.reserve(saved.size());
    for (int i = 0; i < saved.size(); ++i) {
        QVariantMap entry = saved.at(i).toMap();
        const QVariantMap filters = entry.value(QLatin1String(FiltersKey)).toMap();
        entry.insert(QLatin1String(CountKey),
                     m_store->isOpen() ? m_tasks->countMatching(filters) : 0);
        items.append(entry);
    }
    return items;
}

void SmartListModel::reload()
{
    syncItems(computeItems());
}

// The existing row carrying this id at or after \a from, or -1. Ids are never
// reused, and every row before \a from is already settled, so a match can only
// lie ahead.
int SmartListModel::indexOfId(int id, int from) const
{
    for (int i = from; i < m_items.size(); ++i) {
        if (idOf(m_items.at(i)) == id)
            return i;
    }
    return -1;
}

// Rows are matched by id and edited in place, never cleared: a row the user is
// deleting carries its remorse inside its delegate, and a model reset would
// destroy that countdown before it could run. The counts are read again on
// every edit and sync, so a reset would drop a delete already begun on another
// row - which is what left that row stuck dimmed.
void SmartListModel::syncItems(const QVector<QVariantMap> &next)
{
    const int was = m_items.size();
    for (int i = 0; i < next.size(); ++i) {
        const int at = indexOfId(idOf(next.at(i)), i);
        if (at < 0) {
            beginInsertRows(QModelIndex(), i, i);
            m_items.insert(i, next.at(i));
            endInsertRows();
            continue;
        }

        // A match always lies ahead of i, so this only ever moves a row up.
        if (at != i) {
            beginMoveRows(QModelIndex(), at, at, QModelIndex(), i);
            const QVariantMap moved = m_items.at(at);
            m_items.remove(at);
            m_items.insert(i, moved);
            endMoveRows();
        }
        m_items[i] = next.at(i);
        emit dataChanged(index(i), index(i));
    }

    if (m_items.size() > next.size()) {
        beginRemoveRows(QModelIndex(), next.size(), m_items.size() - 1);
        m_items.remove(next.size(), m_items.size() - next.size());
        endRemoveRows();
    }

    if (m_items.size() != was)
        emit countChanged();
}
