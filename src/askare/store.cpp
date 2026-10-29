#include "store.h"

#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>
#include <QVariant>

#include "ics.h"
#include "url.h"

namespace {

QString databasePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/askare.db");
}

QString displayNameFromHref(const QString &href)
{
    const QStringList parts = href.split(QLatin1Char('/'), QString::SkipEmptyParts);
    if (parts.isEmpty())
        return QObject::tr("Tasks");
    return QUrl::fromPercentEncoding(parts.last().toUtf8());
}

QVector<CalendarInfo> calendarsFromTasks(const QSqlDatabase &db)
{
    QVector<CalendarInfo> out;
    QSqlQuery query(db);
    query.exec(QStringLiteral("SELECT DISTINCT calendar_href FROM tasks"
                              " WHERE calendar_href != ''"
                              " ORDER BY calendar_href"));
    while (query.next()) {
        CalendarInfo info;
        info.href = query.value(0).toString();
        info.displayName = displayNameFromHref(info.href);
        out.append(info);
    }
    return out;
}

// Single source for the task column order read back by Store::readTask().
QString taskColumns()
{
    return QStringLiteral("SELECT href, calendar_href, uid, etag, ics, summary,"
                          " description, status, completed, due, priority,"
                          " reminder, reminded_for, parent_uid, dirty, deleted,"
                          " location, url"
                          " FROM tasks");
}

// Open tasks come before completed ones: the task list groups by that flag, and
// a list view only draws a group header where the value differs from the row
// above, so an order that interleaves the two would scatter the headers. The
// comparison is case-insensitive to match Ics::isCompleted(). Within a group a
// missing due date sorts last, so dated tasks lead and undated ones trail.
// A task sits at the top when it names no parent, or names one that is not
// here, so an orphan surfaces instead of vanishing with its missing parent.
QString topLevelClause()
{
    return QStringLiteral(" AND (parent_uid IS NULL OR parent_uid = ''"
                          " OR parent_uid NOT IN"
                          " (SELECT uid FROM tasks WHERE deleted = 0))");
}

QString taskOrder()
{
    return QStringLiteral(" ORDER BY upper(status) = 'COMPLETED',"
                          " due IS NULL OR due = '', due, summary COLLATE NOCASE");
}

void bindTask(QSqlQuery *query, const LocalTask &task)
{
    query->bindValue(QStringLiteral(":href"), task.href);
    query->bindValue(QStringLiteral(":calendar"), task.calendarHref);
    query->bindValue(QStringLiteral(":uid"), task.uid);
    query->bindValue(QStringLiteral(":etag"), task.etag);
    query->bindValue(QStringLiteral(":ics"), task.ics);
    query->bindValue(QStringLiteral(":summary"), task.summary);
    query->bindValue(QStringLiteral(":description"), task.description);
    query->bindValue(QStringLiteral(":location"), task.location);
    query->bindValue(QStringLiteral(":url"), task.url);
    query->bindValue(QStringLiteral(":status"), task.status);
    query->bindValue(QStringLiteral(":completed"), task.completed);
    query->bindValue(QStringLiteral(":due"), task.due);
    query->bindValue(QStringLiteral(":priority"), task.priority);
    query->bindValue(QStringLiteral(":reminder"), task.reminder);
    query->bindValue(QStringLiteral(":reminded"), task.remindedFor);
    query->bindValue(QStringLiteral(":parent"), task.parentUid);
    query->bindValue(QStringLiteral(":dirty"), task.dirty ? 1 : 0);
    query->bindValue(QStringLiteral(":deleted"), task.deleted ? 1 : 0);
}

}

Store::Store(QObject *parent)
    : QObject(parent)
{
}

bool Store::isOpen() const
{
    return m_db.isValid() && m_db.isOpen();
}

bool Store::open()
{
    if (isOpen())
        return true;

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("askare"));
    m_db.setDatabaseName(databasePath());
    if (!m_db.open()) {
        qWarning() << "askare: cannot open" << m_db.databaseName() << m_db.lastError().text();
        return false;
    }
    return migrate();
}

void Store::clear()
{
    if (!m_db.isOpen())
        return;

    QSqlQuery query(m_db);
    query.exec(QStringLiteral("DELETE FROM tasks"));
    query.exec(QStringLiteral("DELETE FROM calendars"));
}

bool Store::migrate()
{
    QSqlQuery query(m_db);
    const bool calendars = query.exec(
        QStringLiteral("CREATE TABLE IF NOT EXISTS calendars ("
                       "href TEXT PRIMARY KEY,"
                       "display_name TEXT,"
                       "ctag TEXT,"
                       "sync_token TEXT)"));
    const bool tasks = query.exec(
        QStringLiteral("CREATE TABLE IF NOT EXISTS tasks ("
                       "href TEXT PRIMARY KEY,"
                       "calendar_href TEXT,"
                       "uid TEXT,"
                       "etag TEXT,"
                       "ics TEXT,"
                       "summary TEXT,"
                       "description TEXT,"
                       "status TEXT,"
                       "completed TEXT,"
                       "due TEXT,"
                       "priority INTEGER,"
                       "reminder INTEGER DEFAULT -1,"
                       "reminded_for TEXT,"
                       "parent_uid TEXT,"
                       "dirty INTEGER DEFAULT 0,"
                       "deleted INTEGER DEFAULT 0)"));
    // Older databases predate these columns; adding one fails harmlessly when present.
    query.exec(QStringLiteral("ALTER TABLE calendars ADD COLUMN color TEXT"));
    query.exec(QStringLiteral("ALTER TABLE tasks ADD COLUMN reminder INTEGER DEFAULT -1"));
    query.exec(QStringLiteral("ALTER TABLE tasks ADD COLUMN reminded_for TEXT"));
    query.exec(QStringLiteral("ALTER TABLE tasks ADD COLUMN parent_uid TEXT"));
    query.exec(QStringLiteral("ALTER TABLE tasks ADD COLUMN location TEXT"));
    query.exec(QStringLiteral("ALTER TABLE tasks ADD COLUMN url TEXT"));
    return calendars && tasks;
}

void Store::replaceCalendars(const QVector<CalendarInfo> &calendars)
{
    if (!isOpen())
        return;

    QSqlQuery clear(m_db);
    clear.exec(QStringLiteral("DELETE FROM calendars"));
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("INSERT INTO calendars"
                                 "(href, display_name, color, ctag, sync_token)"
                                 " VALUES (:href, :name, :color, :ctag, :token)"));
    for (const CalendarInfo &info : calendars) {
        query.bindValue(QStringLiteral(":href"), Url::pathWithSlash(info.href));
        query.bindValue(QStringLiteral(":name"),
                        info.displayName.isEmpty() ? tr("Tasks") : info.displayName);
        query.bindValue(QStringLiteral(":color"), info.color);
        query.bindValue(QStringLiteral(":ctag"), info.ctag);
        query.bindValue(QStringLiteral(":token"), info.syncToken);
        query.exec();
    }
}

QVector<CalendarInfo> Store::calendars() const
{
    QVector<CalendarInfo> out;
    if (!isOpen())
        return out;

    QSqlQuery query(m_db);
    query.exec(QStringLiteral("SELECT href, display_name, color, ctag, sync_token"
                              " FROM calendars ORDER BY display_name COLLATE NOCASE"));
    while (query.next()) {
        CalendarInfo info;
        info.href = query.value(0).toString();
        info.displayName = query.value(1).toString();
        info.color = query.value(2).toString();
        info.ctag = query.value(3).toString();
        info.syncToken = query.value(4).toString();
        info.allowsTodos = true;
        out.append(info);
    }
    if (out.isEmpty())
        return calendarsFromTasks(m_db);
    return out;
}

LocalTask Store::readTask(const QSqlQuery &query) const
{
    LocalTask task;
    task.href = query.value(0).toString();
    task.calendarHref = query.value(1).toString();
    task.uid = query.value(2).toString();
    task.etag = query.value(3).toString();
    task.ics = query.value(4).toString();
    task.summary = query.value(5).toString();
    task.description = query.value(6).toString();
    task.status = query.value(7).toString();
    task.completed = query.value(8).toString();
    task.due = query.value(9).toString();
    task.priority = query.value(10).toInt();
    task.reminder = query.value(11).isNull() ? -1 : query.value(11).toInt();
    task.remindedFor = query.value(12).toString();
    task.parentUid = query.value(13).toString();
    task.dirty = query.value(14).toInt() != 0;
    task.deleted = query.value(15).toInt() != 0;
    task.location = query.value(16).toString();
    task.url = query.value(17).toString();
    return task;
}

QVector<LocalTask> Store::selectTasks(const QString &extra, const QString &calendarHref) const
{
    QVector<LocalTask> out;
    if (!isOpen())
        return out;

    QSqlQuery query(m_db);
    const QString head = taskColumns() + QStringLiteral(" WHERE deleted = 0") + extra;
    if (calendarHref.isEmpty()) {
        query.exec(head + taskOrder());
    } else {
        query.prepare(head + QStringLiteral(" AND calendar_href = :calendar") + taskOrder());
        query.bindValue(QStringLiteral(":calendar"), calendarHref);
        query.exec();
    }
    while (query.next())
        out.append(readTask(query));
    return out;
}

QVector<LocalTask> Store::tasks(const QString &calendarHref) const
{
    return selectTasks(QString(), calendarHref);
}

QVector<LocalTask> Store::topLevelTasks(const QString &calendarHref) const
{
    return selectTasks(topLevelClause(), calendarHref);
}

QVector<LocalTask> Store::children(const QString &parentUid) const
{
    if (parentUid.isEmpty())
        return QVector<LocalTask>();

    QVector<LocalTask> out;
    if (!isOpen())
        return out;

    QSqlQuery query(m_db);
    query.prepare(taskColumns() + QStringLiteral(" WHERE deleted = 0"
                                                 " AND parent_uid = :parent")
                  + taskOrder());
    query.bindValue(QStringLiteral(":parent"), parentUid);
    query.exec();
    while (query.next())
        out.append(readTask(query));
    return out;
}

QVector<LocalTask> Store::descendants(const QString &href) const
{
    QVector<LocalTask> out;
    const LocalTask root = task(href);
    if (root.uid.isEmpty())
        return out;

    QStringList seen;
    QStringList pending;
    seen.append(root.uid);
    pending.append(root.uid);
    while (!pending.isEmpty()) {
        const QVector<LocalTask> next = children(pending.takeFirst());
        for (const LocalTask &child : next) {
            // A malformed cycle must not spin here.
            if (child.uid.isEmpty() || seen.contains(child.uid))
                continue;
            seen.append(child.uid);
            pending.append(child.uid);
            out.append(child);
        }
    }
    return out;
}

QVector<LocalTask> Store::dirtyTasks() const
{
    QVector<LocalTask> out;
    if (!isOpen())
        return out;

    QSqlQuery query(m_db);
    query.exec(taskColumns() + QStringLiteral(" WHERE dirty = 1"));
    while (query.next())
        out.append(readTask(query));
    return out;
}

LocalTask Store::task(const QString &href) const
{
    if (!isOpen())
        return LocalTask();

    QSqlQuery query(m_db);
    query.prepare(taskColumns() + QStringLiteral(" WHERE href = :href"));
    query.bindValue(QStringLiteral(":href"), href);
    if (!query.exec() || !query.next())
        return LocalTask();

    return readTask(query);
}

int Store::openCount() const
{
    if (!isOpen())
        return 0;

    QSqlQuery query(m_db);
    query.exec(QStringLiteral("SELECT COUNT(*) FROM tasks WHERE deleted = 0"
                              " AND status != 'COMPLETED'"));
    return query.next() ? query.value(0).toInt() : 0;
}

int Store::completedCount() const
{
    if (!isOpen())
        return 0;

    QSqlQuery query(m_db);
    query.exec(QStringLiteral("SELECT COUNT(*) FROM tasks WHERE deleted = 0"
                              " AND status = 'COMPLETED'"));
    return query.next() ? query.value(0).toInt() : 0;
}

int Store::currentCount() const
{
    if (!isOpen())
        return 0;

    int count = 0;
    const QVector<LocalTask> items = tasks();
    for (const LocalTask &task : items) {
        if (Ics::isCurrent(task.status, task.due))
            ++count;
    }
    return count;
}

int Store::importantCount() const
{
    if (!isOpen())
        return 0;

    int count = 0;
    const QVector<LocalTask> items = tasks();
    for (const LocalTask &task : items) {
        if (Ics::isImportant(task.status, task.priority))
            ++count;
    }
    return count;
}

LocalTask Store::nextOpen() const
{
    if (!isOpen())
        return LocalTask();

    QSqlQuery query(m_db);
    query.exec(taskColumns() + QStringLiteral(" WHERE deleted = 0"
                                              " AND status != 'COMPLETED' AND due != ''"
                                              " ORDER BY due LIMIT 1"));
    return query.next() ? readTask(query) : LocalTask();
}

void Store::upsertRemoteTask(const QString &calendarHref, const RemoteTask &remote)
{
    const LocalTask existing = task(remote.href);
    if (existing.dirty)
        return;

    const TodoFields fields = Ics::parse(remote.ics);
    LocalTask next;
    next.href = remote.href;
    next.calendarHref = calendarHref;
    next.uid = fields.uid;
    next.etag = remote.etag;
    next.ics = remote.ics;
    next.summary = fields.summary;
    next.description = fields.description;
    next.location = fields.location;
    next.url = fields.url;
    next.status = fields.status;
    next.completed = fields.completed;
    next.due = fields.due;
    next.priority = fields.priority;
    next.reminder = fields.reminder;
    next.parentUid = fields.parentUid;
    // Keep the notified stamp so a sync does not replay a reminder.
    next.remindedFor = existing.remindedFor;
    next.dirty = false;
    next.deleted = false;
    upsertLocal(next);
}

void Store::dropMissing(const QString &calendarHref, const QStringList &keep)
{
    if (!isOpen())
        return;

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT href, dirty FROM tasks"
                                 " WHERE calendar_href = :calendar AND deleted = 0"));
    query.bindValue(QStringLiteral(":calendar"), calendarHref);
    query.exec();
    QSqlQuery drop(m_db);
    drop.prepare(QStringLiteral("DELETE FROM tasks WHERE href = :href"));
    while (query.next()) {
        if (query.value(1).toInt() != 0 || keep.contains(query.value(0).toString()))
            continue;
        drop.bindValue(QStringLiteral(":href"), query.value(0).toString());
        drop.exec();
    }
}

void Store::mergeRemote(const QString &calendarHref, const QVector<RemoteTask> &tasks)
{
    if (!isOpen())
        return;

    QStringList keep;
    for (const RemoteTask &task : tasks) {
        keep.append(task.href);
        upsertRemoteTask(calendarHref, task);
    }
    dropMissing(calendarHref, keep);
}

void Store::upsertLocal(const LocalTask &task)
{
    if (!isOpen())
        return;

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("INSERT OR REPLACE INTO tasks"
                                 "(href, calendar_href, uid, etag, ics, summary,"
                                 " description, status, completed, due, priority,"
                                 " reminder, reminded_for, parent_uid,"
                                 " dirty, deleted, location, url)"
                                 " VALUES (:href, :calendar, :uid, :etag, :ics,"
                                 " :summary, :description, :status, :completed,"
                                 " :due, :priority, :reminder, :reminded, :parent,"
                                 " :dirty, :deleted, :location, :url)"));
    bindTask(&query, task);
    query.exec();
}

void Store::markReminded(const QString &href, const QString &stamp)
{
    if (!isOpen())
        return;

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE tasks SET reminded_for = :stamp"
                                 " WHERE href = :href"));
    query.bindValue(QStringLiteral(":stamp"), stamp);
    query.bindValue(QStringLiteral(":href"), href);
    query.exec();
}

void Store::markClean(const QString &href, const QString &etag)
{
    if (!isOpen())
        return;

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE tasks SET dirty = 0, deleted = 0, etag = :etag"
                                 " WHERE href = :href"));
    query.bindValue(QStringLiteral(":etag"), etag);
    query.bindValue(QStringLiteral(":href"), href);
    query.exec();
}

void Store::markDeleted(const QString &href)
{
    if (!isOpen())
        return;

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE tasks SET deleted = 1, dirty = 1"
                                 " WHERE href = :href"));
    query.bindValue(QStringLiteral(":href"), href);
    query.exec();
}

void Store::remove(const QString &href)
{
    if (!isOpen())
        return;

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM tasks WHERE href = :href"));
    query.bindValue(QStringLiteral(":href"), href);
    query.exec();
}
