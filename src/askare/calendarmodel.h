#ifndef ASKARE_CALENDARMODEL_H
#define ASKARE_CALENDARMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVector>

#include "types.h"

class Store;

class CalendarModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        HrefRole = Qt::UserRole + 1,
        DisplayNameRole,
        OpenCountRole
    };

    explicit CalendarModel(Store *store, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void reload();
    Q_INVOKABLE QString hrefAt(int row) const;

private:
    Store *m_store;
    QVector<CalendarInfo> m_items;
};

#endif
