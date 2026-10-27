#ifndef ASKARE_STORE_H
#define ASKARE_STORE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVector>

#include "types.h"

class Store : public QObject
{
    Q_OBJECT

public:
    explicit Store(QObject *parent = nullptr);

    bool open();
    bool isOpen() const;
    void clear();

    void replaceCalendars(const QVector<CalendarInfo> &calendars);
    QVector<CalendarInfo> calendars() const;

    void mergeRemote(const QString &calendarHref, const QVector<RemoteTask> &tasks);
    // Every task, whatever its place in the tree. Reminders and the cover
    // counts rely on this staying flat.
    QVector<LocalTask> tasks(const QString &calendarHref = QString()) const;
    // Tasks with no parent, plus any whose parent is no longer here.
    QVector<LocalTask> topLevelTasks(const QString &calendarHref = QString()) const;
    QVector<LocalTask> children(const QString &parentUid) const;
    QVector<LocalTask> descendants(const QString &href) const;
    QVector<LocalTask> dirtyTasks() const;
    LocalTask task(const QString &href) const;
    int openCount() const;
    int currentCount() const;
    int importantCount() const;
    int completedCount() const;
    LocalTask nextOpen() const;

    void upsertLocal(const LocalTask &task);
    void markReminded(const QString &href, const QString &stamp);
    void markClean(const QString &href, const QString &etag);
    void markDeleted(const QString &href);
    void remove(const QString &href);

private:
    bool migrate();
    QVector<LocalTask> selectTasks(const QString &extra, const QString &calendarHref) const;
    LocalTask readTask(const QSqlQuery &query) const;
    void upsertRemoteTask(const QString &calendarHref, const RemoteTask &task);
    void dropMissing(const QString &calendarHref, const QStringList &keep);

    QSqlDatabase m_db;
};

#endif
