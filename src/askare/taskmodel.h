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
    Q_PROPERTY(QString tag READ tag NOTIFY tagChanged)
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
        GroupRole,
        ChildCountRole,
        ChildDoneCountRole
    };

    explicit TaskModel(Store *store, QObject *parent = nullptr);

    QString calendarHref() const { return m_calendarHref; }
    QString filter() const { return m_filter; }
    QString tag() const { return m_tag; }
    QString title() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void showAll();
    Q_INVOKABLE void showCurrent();
    Q_INVOKABLE void showImportant();
    Q_INVOKABLE void showCompleted();
    Q_INVOKABLE void showCalendar(const QString &href, const QString &name);
    Q_INVOKABLE void showTag(const QString &tag);
    // Whether the mixed views keep completed tasks; the dedicated completed
    // view ignores this.
    void setShowCompleted(bool show);
    // The same for cancelled tasks, which the actionable views never show.
    void setShowCancelled(bool show);
    void reload();

signals:
    void calendarHrefChanged();
    void filterChanged();
    void tagChanged();
    void titleChanged();

private:
    // Children and completed children of one uid.
    typedef QPair<int, int> ChildTally;

    void apply(const QString &filter, const QString &href, const QString &name);
    bool isMixedView() const;
    ChildTally tallyOf(const QString &uid) const;

    Store *m_store;
    QString m_calendarHref;
    QString m_calendarName;
    QString m_filter;
    QString m_tag;
    bool m_showCompleted;
    bool m_showCancelled;
    QVector<LocalTask> m_items;
    QHash<QString, ChildTally> m_tallies;
    QHash<QString, QString> m_colors;
};

#endif
