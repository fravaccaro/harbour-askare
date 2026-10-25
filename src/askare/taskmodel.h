#ifndef ASKARE_TASKMODEL_H
#define ASKARE_TASKMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVector>

#include "types.h"

class Store;

class TaskModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString calendarHref READ calendarHref WRITE setCalendarHref
               NOTIFY calendarHrefChanged)

public:
    enum Roles {
        HrefRole = Qt::UserRole + 1,
        CalendarHrefRole,
        UidRole,
        SummaryRole,
        DescriptionRole,
        StatusRole,
        DueRole,
        DueDateRole,
        PriorityRole,
        CompletedRole,
        DoneRole
    };

    explicit TaskModel(Store *store, QObject *parent = nullptr);

    QString calendarHref() const { return m_calendarHref; }
    void setCalendarHref(const QString &href);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void reload();

signals:
    void calendarHrefChanged();

private:
    Store *m_store;
    QString m_calendarHref;
    QVector<LocalTask> m_items;
};

#endif
