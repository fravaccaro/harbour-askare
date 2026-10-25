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
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        HrefRole = Qt::UserRole + 1,
        DisplayNameRole,
        ColorRole,
        OpenCountRole
    };

    explicit CalendarModel(Store *store, QObject *parent = nullptr);

    int count() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void reload();
    Q_INVOKABLE QString hrefAt(int row) const;
    Q_INVOKABLE int indexOfHref(const QString &href) const;
    Q_INVOKABLE QString nameOfHref(const QString &href) const;

signals:
    void countChanged();

private:
    Store *m_store;
    QVector<CalendarInfo> m_items;
};

#endif
