#include "calendarmodel.h"

#include "ics.h"
#include "store.h"
#include "url.h"

CalendarModel::CalendarModel(Store *store, QObject *parent)
    : QAbstractListModel(parent)
    , m_store(store)
{
}

int CalendarModel::count() const
{
    return m_items.size();
}

int CalendarModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QHash<int, QByteArray> CalendarModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(HrefRole, "href");
    roles.insert(DisplayNameRole, "displayName");
    roles.insert(ColorRole, "color");
    roles.insert(OpenCountRole, "openCount");
    return roles;
}

QVariant CalendarModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return QVariant();

    const CalendarInfo &info = m_items.at(index.row());
    if (role == HrefRole)
        return info.href;
    if (role == DisplayNameRole)
        return info.displayName;
    if (role == ColorRole)
        return info.color;
    if (role == OpenCountRole) {
        int count = 0;
        const QVector<LocalTask> tasks = m_store->tasks(info.href);
        for (const LocalTask &task : tasks) {
            if (!Ics::isCompleted(task.status))
                ++count;
        }
        return count;
    }
    return QVariant();
}

void CalendarModel::reload()
{
    beginResetModel();
    m_items = m_store->calendars();
    endResetModel();
    emit countChanged();
}

QString CalendarModel::hrefAt(int row) const
{
    if (row < 0 || row >= m_items.size())
        return QString();
    return m_items.at(row).href;
}

int CalendarModel::indexOfHref(const QString &href) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (Url::pathWithSlash(m_items.at(i).href) == Url::pathWithSlash(href))
            return i;
    }
    return 0;
}

QString CalendarModel::nameOfHref(const QString &href) const
{
    for (const CalendarInfo &info : m_items) {
        if (Url::pathWithSlash(info.href) == Url::pathWithSlash(href))
            return info.displayName;
    }
    return QString();
}
