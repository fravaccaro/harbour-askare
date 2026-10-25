#ifndef ASKARE_TASKMODEL_H
#define ASKARE_TASKMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QPair>
#include <QString>
#include <QVector>

#include "types.h"

class Store;

class TaskModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString calendarHref READ calendarHref NOTIFY calendarHrefChanged)
    Q_PROPERTY(QString filter READ filter NOTIFY filterChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)

public:
    enum Roles {
        HrefRole = Qt::UserRole + 1,
        CalendarHrefRole,
        CalendarColorRole,
        UidRole,
        SummaryRole,
        DescriptionRole,
        StatusRole,
        DueRole,
        DueDateRole,
        DueTimeRole,
        ReminderRole,
        PriorityRole,
        PercentRole,
        CompletedRole,
        DoneRole,
        ChildCountRole,
        ChildDoneCountRole
    };

    explicit TaskModel(Store *store, QObject *parent = nullptr);

    QString calendarHref() const { return m_calendarHref; }
    QString filter() const { return m_filter; }
    QString title() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void showAll();
    Q_INVOKABLE void showCurrent();
    Q_INVOKABLE void showCompleted();
    Q_INVOKABLE void showCalendar(const QString &href, const QString &name);
    void reload();

signals:
    void calendarHrefChanged();
    void filterChanged();
    void titleChanged();

private:
    // Children and completed children of one uid.
    typedef QPair<int, int> ChildTally;

    void apply(const QString &filter, const QString &href, const QString &name);
    ChildTally tallyOf(const QString &uid) const;

    Store *m_store;
    QString m_calendarHref;
    QString m_calendarName;
    QString m_filter;
    QVector<LocalTask> m_items;
    QHash<QString, ChildTally> m_tallies;
    QHash<QString, QString> m_colors;
};

#endif
