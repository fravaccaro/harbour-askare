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

    // Returns the remote versions it would not write because the task carries
    // an edit of its own made against a version the server no longer holds.
    QVector<RemoteTask> mergeRemote(const QString &calendarHref,
                                    const QVector<RemoteTask> &tasks);
    // Every task, whatever its place in the tree. Reminders and the cover
    // counts rely on this staying flat.
    QVector<LocalTask> tasks(const QString &calendarHref = QString()) const;
    // Tasks with no parent, plus any whose parent is no longer here.
    QVector<LocalTask> topLevelTasks(const QString &calendarHref = QString()) const;
    QVector<LocalTask> children(const QString &parentUid) const;
    QVector<LocalTask> descendants(const QString &href) const;
    QVector<LocalTask> dirtyTasks() const;
    LocalTask task(const QString &href) const;
    QVector<TagInfo> tagInfos() const;
    int openCount() const;
    int currentCount() const;
    int todayCount() const;
    int importantCount() const;
    int completedCount() const;
    LocalTask nextOpen() const;

    void upsertLocal(const LocalTask &task);
    void markReminded(const QString &href, const QString &stamp);
    void markClean(const QString &href, const QString &etag);
    // Leaves the task owed, carrying the version the server asked for.
    void setEtag(const QString &href, const QString &etag);
    // Writes the server's version over a task changed here.
    void acceptRemote(const QString &calendarHref, const RemoteTask &remote);
    void markDeleted(const QString &href);
    void remove(const QString &href);

private:
    bool migrate();
    QVector<LocalTask> selectTasks(const QString &extra, const QString &calendarHref) const;
    LocalTask readTask(const QSqlQuery &query) const;
    bool clashesWith(const RemoteTask &remote) const;
    void upsertRemoteTask(const QString &calendarHref, const RemoteTask &task);
    void dropMissing(const QString &calendarHref, const QStringList &keep);

    QSqlDatabase m_db;
};

#endif
