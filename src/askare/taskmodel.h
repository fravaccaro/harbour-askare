#ifndef ASKARE_TASKMODEL_H
#define ASKARE_TASKMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QPair>
#include <QString>
#include <QVariantMap>
#include <QVector>

#include "types.h"

class Store;

class TaskModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString calendarHref READ calendarHref NOTIFY calendarHrefChanged)
    Q_PROPERTY(QString filter READ filter NOTIFY filterChanged)
    Q_PROPERTY(int smartListId READ smartListId NOTIFY smartListIdChanged)
    Q_PROPERTY(QString tag READ tag NOTIFY tagChanged)
    Q_PROPERTY(QString search READ search WRITE setSearch NOTIFY searchChanged)
    Q_PROPERTY(QVariantMap filters READ filters WRITE setFilters NOTIFY filtersChanged)
    Q_PROPERTY(bool filtersActive READ filtersActive NOTIFY filtersChanged)
    Q_PROPERTY(int filterCount READ filterCount NOTIFY filtersChanged)
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
    // The saved set the smart view is showing, or -1 in any other view.
    int smartListId() const { return m_smartListId; }
    QString tag() const { return m_tag; }
    QString search() const { return m_search; }
    QVariantMap filters() const { return m_filters; }
    // Whether any filter narrows the search, so an empty query still pulls from
    // every list once a filter is on.
    bool filtersActive() const;
    // How many of the filters are set, which the search page shows beside its
    // Filters row.
    int filterCount() const;
    QString title() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void showAll();
    Q_INVOKABLE void showCurrent();
    Q_INVOKABLE void showToday();
    Q_INVOKABLE void showImportant();
    Q_INVOKABLE void showCompleted();
    Q_INVOKABLE void showCalendar(const QString &href, const QString &name);
    Q_INVOKABLE void showTag(const QString &tag);
    // A saved search as a view of its own. It reads like the search it was
    // saved from, so the query being typed now is dropped.
    Q_INVOKABLE void showSmart(int id, const QString &name, const QVariantMap &filters);
    // How many tasks a saved search keeps, for the row that offers it.
    Q_INVOKABLE int countMatching(const QVariantMap &filters) const;
    // The query and the filters as one set, which is what a smart list saves.
    // The query rides along under a key of its own.
    Q_INVOKABLE QVariantMap searchSet() const;
    // Looks through every task in every list. An empty query shows the view
    // in full again, unless a filter still narrows it.
    void setSearch(const QString &text);
    // Narrows the search to the tasks every active filter accepts. An empty map
    // clears them. The filters hold whether or not a query is typed.
    void setFilters(const QVariantMap &filters);
    // Whether the mixed views keep completed tasks; the dedicated completed
    // view ignores this.
    void setShowCompleted(bool show);
    // The same for cancelled tasks, which the actionable views never show.
    void setShowCancelled(bool show);
    void reload();

signals:
    void calendarHrefChanged();
    void filterChanged();
    void smartListIdChanged();
    void tagChanged();
    void searchChanged();
    void filtersChanged();
    void titleChanged();

private:
    // Children and completed children of one uid.
    typedef QPair<int, int> ChildTally;

    void apply(const QString &filter, const QString &href, const QString &name);
    void forgetSmartList();
    bool isMixedView() const;
    QVector<LocalTask> filtered() const;
    QVector<LocalTask> matchedBy(const QString &query, const QVariantMap &filters) const;
    QVector<LocalTask> searched() const;
    QVector<LocalTask> smartFiltered() const;
    // Drops completed and cancelled tasks the mixed-view settings hide.
    QVector<LocalTask> withSettings(const QVector<LocalTask> &tasks) const;
    ChildTally tallyOf(const QString &uid) const;

    Store *m_store;
    QString m_calendarHref;
    QString m_calendarName;
    QString m_filter;
    QString m_tag;
    QString m_search;
    QVariantMap m_filters;
    // The smart view's own saved search, query and all, kept apart from the
    // live one so leaving a search does not empty the view underneath it.
    QVariantMap m_viewFilters;
    int m_smartListId;
    bool m_showCompleted;
    bool m_showCancelled;
    QVector<LocalTask> m_items;
    QHash<QString, ChildTally> m_tallies;
    QHash<QString, QString> m_colors;
};

#endif
