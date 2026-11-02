#include "session.h"

#include "calendarmodel.h"
#include "ics.h"
#include "markdown.h"
#include "taskmodel.h"
#include "url.h"

namespace {

// Where a task of this uid lives inside a collection.
QString hrefIn(const QString &calendarHref, const QString &uid)
{
    return Url::pathWithSlash(calendarHref) + uid + QStringLiteral(".ics");
}

// A QStringList reaches QML as a sequence wrapper, which boxes as one element
// when carried through a plain object. A QVariantList arrives as a real array.
QVariantList toVariantList(const QStringList &values)
{
    QVariantList out;
    for (const QString &value : values)
        out.append(value);
    return out;
}

// Only a field the two versions disagree on is worth putting to the user.
// Priority, status and reminder travel as they are stored, since the words for
// them live in the QML objects that already hold that mapping.
void appendDifference(QVariantList *rows, const QString &field, const QString &label,
                      const QVariant &mine, const QVariant &theirs)
{
    if (mine == theirs)
        return;

    QVariantMap row;
    row.insert(QStringLiteral("field"), field);
    row.insert(QStringLiteral("label"), label);
    row.insert(QStringLiteral("mine"), mine);
    row.insert(QStringLiteral("theirs"), theirs);
    rows->append(row);
}

// A moment as the pages word it: the date, and the time when there is one.
QString momentText(const QString &due)
{
    const QString date = Ics::dateFromDue(due);
    if (date.isEmpty())
        return QString();

    const QString time = Ics::timeFromDue(due);
    return time.isEmpty() ? date : date + QLatin1Char(' ') + time;
}

void ensureIdentity(LocalTask *task, const QString &calendarHref)
{
    if (!task->href.isEmpty())
        return;

    task->uid = Ics::newUid();
    task->href = hrefIn(calendarHref, task->uid);
    task->calendarHref = Url::pathWithSlash(calendarHref);
}

// The same task under another collection: a resource the server has not seen,
// so it is uploaded rather than updated. The uid stays, which keeps the
// RELATED-TO links inside a moved subtree pointing at the right parent, and so
// does the reminded stamp, so carrying a task over does not replay a reminder.
LocalTask relocated(const LocalTask &task, const QString &calendarHref)
{
    LocalTask next = task;
    next.calendarHref = Url::pathWithSlash(calendarHref);
    next.href = hrefIn(calendarHref, task.uid);
    next.etag = QString();
    next.dirty = true;
    next.deleted = false;
    return next;
}

// A task carried out of its collection cannot keep a parent it leaves behind:
// RELATED-TO resolves by uid only within one collection. So it becomes a task
// in its own right, in the row and in the ICS alike.
LocalTask detached(const LocalTask &task)
{
    LocalTask next = task;
    next.parentUid = QString();
    TodoFields fields = Ics::parse(next.ics);
    fields.parentUid = QString();
    next.ics = Ics::rewrite(next.ics, fields);
    return next;
}

}

Session::Session(Config *config, Store *store, CalendarModel *calendars,
                 TaskModel *tasks, QObject *parent)
    : QObject(parent)
    , m_config(config)
    , m_store(store)
    , m_calendars(calendars)
    , m_tasks(tasks)
    , m_busy(false)
    , m_pushOnly(false)
    , m_pullAfterConflict(false)
    , m_uploaded(0)
    , m_syncedOnStart(false)
{
    m_syncTimer.setSingleShot(false);
    connect(&m_syncTimer, &QTimer::timeout, this, &Session::sync);
    connect(&m_login, &LoginFlow::loginUrlReady, this, &Session::onLoginUrl);
    connect(&m_login, &LoginFlow::finished, this, &Session::onLoggedIn);
    connect(&m_login, &LoginFlow::failed, this, &Session::onLoginFailed);
    connect(&m_dav, &DavClient::principalReady, this, &Session::onPrincipal);
    connect(&m_dav, &DavClient::homeReady, this, &Session::onHome);
    connect(&m_dav, &DavClient::calendarsReady, this, &Session::onCalendars);
    connect(&m_dav, &DavClient::todosReady, this, &Session::onTodos);
    connect(&m_dav, &DavClient::uploaded, this, &Session::onUploaded);
    connect(&m_dav, &DavClient::removed, this, &Session::onRemoved);
    connect(&m_dav, &DavClient::failed, this, &Session::onDavFailed);
    connect(&m_dav, &DavClient::conflicted, this, &Session::onConflicted);
    connect(m_config, &Config::readyChanged, this, &Session::readyChanged);
    connect(m_config, &Config::configuredChanged, this, &Session::onConfiguredChanged);
    connect(m_config, &Config::secretsErrorChanged, this, &Session::secretsErrorChanged);
    connect(m_config, &Config::serverUrlChanged, this, &Session::serverUrlChanged);
    connect(m_config, &Config::usernameChanged, this, &Session::usernameChanged);
    connect(m_config, &Config::ignoreSslErrorsChanged, this, &Session::ignoreSslErrorsChanged);
    connect(m_config, &Config::syncOnStartChanged, this, &Session::syncOnStartChanged);
    connect(m_config, &Config::syncAfterPushChanged, this, &Session::syncAfterPushChanged);
    connect(m_config, &Config::syncIntervalChanged, this, &Session::syncIntervalChanged);
    connect(m_config, &Config::syncIntervalChanged, this, &Session::applySyncTimer);
    connect(m_config, &Config::showCompletedChanged, this,
            &Session::showCompletedTasksChanged);
    connect(m_config, &Config::showCompletedChanged, this, &Session::applyShowCompleted);
    connect(m_config, &Config::showCancelledChanged, this,
            &Session::showCancelledTasksChanged);
    connect(m_config, &Config::showCancelledChanged, this, &Session::applyShowCancelled);
    connect(m_config, &Config::readyChanged, this, &Session::syncIfStartupWanted);
    m_store->open();
    applyShowCompleted();
    applyShowCancelled();
    if (m_config->isReady() && m_config->isConfigured())
        applyCredentials();
    applySyncTimer();
    syncIfStartupWanted();
}

bool Session::isReady() const
{
    return m_config->isReady();
}

bool Session::isConfigured() const
{
    return m_config->isConfigured();
}

QString Session::secretsError() const
{
    return m_config->secretsError();
}

QString Session::serverUrl() const
{
    return m_config->serverUrl();
}

QString Session::username() const
{
    return m_config->username();
}

bool Session::ignoreSslErrors() const
{
    return m_config->ignoreSslErrors();
}

void Session::setIgnoreSslErrors(bool ignore)
{
    m_config->setIgnoreSslErrors(ignore);
}

bool Session::syncOnStart() const
{
    return m_config->syncOnStart();
}

void Session::setSyncOnStart(bool enabled)
{
    m_config->setSyncOnStart(enabled);
}

int Session::syncInterval() const
{
    return m_config->syncInterval();
}

void Session::setSyncInterval(int minutes)
{
    m_config->setSyncInterval(minutes);
}

bool Session::syncAfterPush() const
{
    return m_config->syncAfterPush();
}

void Session::setSyncAfterPush(bool enabled)
{
    m_config->setSyncAfterPush(enabled);
}

bool Session::showCompletedTasks() const
{
    return m_config->showCompleted();
}

void Session::setShowCompletedTasks(bool show)
{
    m_config->setShowCompleted(show);
}

void Session::applyShowCompleted()
{
    m_tasks->setShowCompleted(m_config->showCompleted());
}

bool Session::showCancelledTasks() const
{
    return m_config->showCancelled();
}

void Session::setShowCancelledTasks(bool show)
{
    m_config->setShowCancelled(show);
}

void Session::applyShowCancelled()
{
    m_tasks->setShowCancelled(m_config->showCancelled());
}

void Session::applySyncTimer()
{
    const int minutes = m_config->syncInterval();
    if (minutes <= 0 || !m_config->isConfigured()) {
        m_syncTimer.stop();
        return;
    }

    m_syncTimer.setInterval(minutes * 60 * 1000);
    m_syncTimer.start();
}

void Session::syncIfStartupWanted()
{
    if (m_syncedOnStart || !m_config->isReady() || !m_config->isConfigured())
        return;
    if (!m_config->syncOnStart())
        return;

    m_syncedOnStart = true;
    sync();
}

int Session::openCount() const
{
    return m_store->isOpen() ? m_store->openCount() : 0;
}

int Session::currentCount() const
{
    return m_store->isOpen() ? m_store->currentCount() : 0;
}

int Session::importantCount() const
{
    return m_store->isOpen() ? m_store->importantCount() : 0;
}

QVariantList Session::tags() const
{
    QVariantList out;
    const QVector<TagInfo> infos = m_store->tagInfos();
    for (const TagInfo &info : infos) {
        QVariantMap entry;
        entry.insert(QStringLiteral("name"), info.name);
        entry.insert(QStringLiteral("count"), info.openCount);
        out.append(entry);
    }
    return out;
}

int Session::completedCount() const
{
    return m_store->isOpen() ? m_store->completedCount() : 0;
}

QString Session::nextDueSummary() const
{
    return m_store->isOpen() ? m_store->nextOpen().summary : QString();
}

void Session::setBusy(bool busy)
{
    if (busy == m_busy)
        return;

    m_busy = busy;
    emit busyChanged();
}

void Session::setError(const QString &error)
{
    if (error == m_error)
        return;

    m_error = error;
    emit errorMessageChanged();
}

void Session::reloadModels()
{
    m_calendars->reload();
    m_tasks->reload();
    emit coverChanged();
    emit tasksChanged();
}

void Session::applyCredentials()
{
    m_store->open();
    m_dav.setServer(m_config->serverUrl());
    m_dav.setCredentials(m_config->username(), m_config->password());
    m_dav.setIgnoreSslErrors(m_config->ignoreSslErrors());
    reloadModels();
}

void Session::onConfiguredChanged()
{
    emit configuredChanged();
    if (m_config->isConfigured())
        applyCredentials();
    applySyncTimer();
    syncIfStartupWanted();
}

void Session::startLogin(const QString &serverUrl)
{
    setError(QString());
    setBusy(true);
    m_config->setServerUrl(serverUrl);
    m_login.start(serverUrl, m_config->ignoreSslErrors());
}

void Session::cancelLogin()
{
    m_login.cancel();
    setBusy(false);
    if (!m_loginUrl.isEmpty()) {
        m_loginUrl.clear();
        emit loginUrlChanged();
    }
}

void Session::logout()
{
    cancelLogin();
    m_syncTimer.stop();
    m_syncedOnStart = false;
    m_conflicts.clear();
    m_refused.clear();
    emit conflictsChanged();
    m_store->clear();
    m_config->clearCredentials();
    m_tasks->showAll();
    reloadModels();
}

void Session::reloadSecrets()
{
    m_config->reloadSecrets();
}

void Session::onLoginUrl(const QString &url)
{
    setBusy(false);
    m_loginUrl = url;
    emit loginUrlChanged();
    emit loginUrlReady(url);
}

void Session::onLoggedIn(const QString &server, const QString &user,
                         const QString &password)
{
    m_config->setServerUrl(server);
    m_config->setUsername(user);
    m_config->setPassword(password);
    applyCredentials();
    emit loginSucceeded();
    m_syncedOnStart = true;
    sync();
}

// The sign-in pages show the reason themselves, so it does not also become a
// notice over whatever the user is looking at.
void Session::onLoginFailed(const QString &error)
{
    setBusy(false);
    emit loginFailed(error);
}

void Session::sync()
{
    if (!m_config->isConfigured() || m_busy)
        return;

    m_pushOnly = false;
    m_uploaded = 0;
    setError(QString());
    setBusy(true);
    applyCredentials();
    m_dav.discoverPrincipal();
}

void Session::onPrincipal(const QString &href)
{
    m_dav.discoverHome(href);
}

void Session::onHome(const QString &href)
{
    m_dav.listCalendars(href);
}

void Session::onCalendars(const QVector<CalendarInfo> &calendars)
{
    m_store->replaceCalendars(calendars);
    m_pendingCalendars = calendars;
    fetchNextCalendar();
}

void Session::fetchNextCalendar()
{
    if (m_pendingCalendars.isEmpty()) {
        m_pendingUploads = withoutConflicts(m_store->dirtyTasks());
        pushNext();
        return;
    }

    const CalendarInfo info = m_pendingCalendars.takeFirst();
    m_currentCalendarHref = Url::pathWithSlash(info.href);
    m_dav.fetchTodos(m_currentCalendarHref);
}

void Session::onTodos(const QVector<RemoteTask> &tasks)
{
    const QVector<RemoteTask> clashes = m_store->mergeRemote(m_currentCalendarHref, tasks);
    for (const RemoteTask &remote : clashes)
        noteConflict(remote);

    fetchNextCalendar();
}

// A task changed on both sides. The merge left the edit made here alone, so
// the two versions are still whole and the user can be asked which to keep.
void Session::noteConflict(const RemoteTask &remote)
{
    // Whatever becomes of the task now supersedes any earlier refusal of it.
    m_refused.remove(remote.href);
    const LocalTask mine = m_store->task(remote.href);
    const TodoFields theirs = Ics::parse(remote.ics);
    // Both sides arrived at the same task: nothing to choose between, and
    // nothing worth saying about it either.
    if (differencesBetween(Ics::parse(mine.ics), theirs).isEmpty()) {
        m_store->markClean(remote.href, remote.etag);
        return;
    }

    // Deleted here. The delete is the answer the user has already given; it
    // only wants the version the server asks for to go through.
    if (mine.deleted) {
        m_store->setEtag(remote.href, remote.etag);
        return;
    }

    const int at = indexOfConflict(remote.href);
    // Asked already and still unanswered: the pair is kept current rather than
    // the same question put twice.
    if (at >= 0) {
        m_conflicts[at] = qMakePair(mine, remote);
        return;
    }

    m_conflicts.append(qMakePair(mine, remote));
    emit conflictsChanged();
    setError(tr("%1 changed on the server as well. Choose which version to keep.")
             .arg(mine.summary));
}

int Session::indexOfConflict(const QString &href) const
{
    for (int i = 0; i < m_conflicts.size(); ++i) {
        if (m_conflicts.at(i).first.href == href)
            return i;
    }

    return -1;
}

// Takes the pair out, so an answer settles a task once and the next sync is
// free to raise it again.
bool Session::takeConflict(const QString &href, QPair<LocalTask, RemoteTask> *pair)
{
    const int at = indexOfConflict(href);
    if (at < 0)
        return false;

    *pair = m_conflicts.at(at);
    m_conflicts.remove(at);
    return true;
}

// A task waiting to be settled must not go out: the version it carries is the
// one the server has already refused, and sending it only earns another 412.
QVector<LocalTask> Session::withoutConflicts(const QVector<LocalTask> &tasks) const
{
    if (m_conflicts.isEmpty() && m_refused.isEmpty())
        return tasks;

    QVector<LocalTask> out;
    for (const LocalTask &task : tasks) {
        if (indexOfConflict(task.href) < 0 && !m_refused.contains(task.href))
            out.append(task);
    }
    return out;
}

// A parent is named by uid, which is no answer to put to the user. A parent
// this device has not got stays named by its uid rather than by nothing.
QString Session::taskName(const QString &uid) const
{
    if (uid.isEmpty())
        return QString();

    const QVector<LocalTask> tasks = m_store->tasks();
    for (const LocalTask &task : tasks) {
        if (task.uid == uid)
            return task.summary;
    }

    return uid;
}

// Every field a task carries that the user set, so a version differing only in
// something not listed here is taken for identical and settled without asking.
// The completion stamp and the uid are left out: both follow from the rest.
QVariantList Session::differencesBetween(const TodoFields &mine,
                                         const TodoFields &theirs) const
{
    QVariantList rows;
    appendDifference(&rows, QStringLiteral("summary"), tr("Name"),
                     mine.summary, theirs.summary);
    appendDifference(&rows, QStringLiteral("status"), tr("Status"),
                     mine.status, theirs.status);
    appendDifference(&rows, QStringLiteral("priority"), tr("Priority"),
                     mine.priority, theirs.priority);
    appendDifference(&rows, QStringLiteral("percent"), tr("Completion"),
                     mine.percent, theirs.percent);
    appendDifference(&rows, QStringLiteral("start"), tr("Start"),
                     momentText(mine.start), momentText(theirs.start));
    appendDifference(&rows, QStringLiteral("due"), tr("Due"),
                     momentText(mine.due), momentText(theirs.due));
    appendDifference(&rows, QStringLiteral("reminder"), tr("Reminder"),
                     mine.reminder, theirs.reminder);
    appendDifference(&rows, QStringLiteral("tags"), tr("Tags"),
                     mine.tags.join(QStringLiteral(", ")),
                     theirs.tags.join(QStringLiteral(", ")));
    appendDifference(&rows, QStringLiteral("location"), tr("Location"),
                     mine.location, theirs.location);
    appendDifference(&rows, QStringLiteral("url"), tr("Address"), mine.url, theirs.url);
    appendDifference(&rows, QStringLiteral("description"), tr("Notes"),
                     mine.description, theirs.description);
    appendDifference(&rows, QStringLiteral("parent"), tr("Subtask of"),
                     taskName(mine.parentUid), taskName(theirs.parentUid));
    return rows;
}

QVariantMap Session::conflictRow(const QPair<LocalTask, RemoteTask> &pair) const
{
    QVariantMap row;
    row.insert(QStringLiteral("href"), pair.first.href);
    row.insert(QStringLiteral("summary"), pair.first.summary);
    row.insert(QStringLiteral("differences"),
               differencesBetween(Ics::parse(pair.first.ics), Ics::parse(pair.second.ics)));
    return row;
}

QVariantList Session::conflicts() const
{
    QVariantList list;
    list.reserve(m_conflicts.size());
    for (int i = 0; i < m_conflicts.size(); ++i)
        list.append(conflictRow(m_conflicts.at(i)));

    return list;
}

// The task stays owed and takes on the version the server answered with, so
// what goes out is a conditional write again rather than one that overwrites
// whatever is there by then.
void Session::resolveKeepMine(const QString &href)
{
    QPair<LocalTask, RemoteTask> pair;
    if (!takeConflict(href, &pair))
        return;

    m_store->setEtag(href, pair.second.etag);
    emit conflictsChanged();
    pushDirty();
}

void Session::resolveKeepTheirs(const QString &href)
{
    QPair<LocalTask, RemoteTask> pair;
    if (!takeConflict(href, &pair))
        return;

    m_store->acceptRemote(pair.first.calendarHref, pair.second);
    reloadModels();
    emit conflictsChanged();
}

void Session::pushNext()
{
    if (m_pendingUploads.isEmpty()) {
        finishSync();
        return;
    }

    const LocalTask task = m_pendingUploads.takeFirst();
    if (task.deleted)
        m_dav.remove(task.href, task.etag);
    else
        m_dav.upload(task.href, Ics::normalize(task.ics), task.etag);
}

void Session::finishSync()
{
    // Only a run that uploaded something may pull afterwards: a task the server
    // keeps rejecting stays dirty, and pushDirty() below would re-queue it.
    const bool pullAfterPush = m_pushOnly && m_uploaded > 0 && m_config->syncAfterPush();
    // A refused write earns a pull whatever the settings say: the pull is what
    // reads the version the server holds, and so what raises the question.
    const bool pull = pullAfterPush || m_pullAfterConflict;
    m_pullAfterConflict = false;
    setBusy(false);
    reloadModels();
    if (m_syncTimer.isActive())
        m_syncTimer.start();
    pushDirty();
    if (pull && !m_busy)
        QTimer::singleShot(0, this, &Session::sync);
}

// A write refused for the version it names. The task keeps its edit and stays
// owed; the pull this earns reads what the server holds and puts the two side
// by side. Until then the task is held back, so a refusal the pull turns out
// not to explain is offered once and not again in a circle. The rest of what
// is owed is still worth sending, so the run carries on.
void Session::onConflicted(const QString &href)
{
    m_refused.insert(href);
    m_pullAfterConflict = true;
    if (!m_pendingUploads.isEmpty()) {
        pushNext();
        return;
    }

    finishSync();
}

void Session::onUploaded(const QString &href, const QString &etag)
{
    ++m_uploaded;
    m_store->markClean(href, etag);
    pushNext();
}

void Session::onRemoved(const QString &href)
{
    ++m_uploaded;
    m_store->remove(href);
    pushNext();
}

void Session::onDavFailed(const QString &error)
{
    if (!m_pendingUploads.isEmpty() || !m_pendingCalendars.isEmpty()) {
        setError(error);
        m_pendingCalendars.clear();
        pushNext();
        return;
    }

    setBusy(false);
    setError(error);
    reloadModels();
}

LocalTask Session::editedTask(const QVariantMap &input) const
{
    const QString startDate = input.value(QStringLiteral("startDate")).toString();
    const QString dueDate = input.value(QStringLiteral("dueDate")).toString();
    // Status drives completion: a task marked done always reads as 100%, and
    // the percent otherwise names the status when the caller leaves it unset.
    const int rawPercent = input.value(QStringLiteral("percent")).toInt();
    const QString status = input.contains(QStringLiteral("status"))
            ? input.value(QStringLiteral("status")).toString()
            : Ics::statusFromPercent(rawPercent);
    const int percent = Ics::isCompleted(status) ? 100 : rawPercent;
    LocalTask task = m_store->task(input.value(QStringLiteral("href")).toString());
    ensureIdentity(&task, input.value(QStringLiteral("calendarHref")).toString());

    TodoFields fields = Ics::parse(task.ics);
    fields.uid = task.uid.isEmpty() ? fields.uid : task.uid;
    fields.summary = input.value(QStringLiteral("summary")).toString();
    fields.description = input.value(QStringLiteral("description")).toString();
    fields.location = input.value(QStringLiteral("location")).toString();
    fields.url = input.value(QStringLiteral("url")).toString();
    fields.tags = input.value(QStringLiteral("tags")).toStringList();
    fields.start = startDate.isEmpty()
            ? QString()
            : Ics::dueFromDateTime(startDate,
                                   input.value(QStringLiteral("startTime")).toString());
    fields.due = dueDate.isEmpty()
            ? QString()
            : Ics::dueFromDateTime(dueDate, input.value(QStringLiteral("dueTime")).toString());
    // A reminder is relative to the due time, so it needs one.
    fields.reminder = fields.due.isEmpty()
            ? -1 : input.value(QStringLiteral("reminder"), -1).toInt();
    fields.parentUid = input.value(QStringLiteral("parentUid")).toString();
    fields.priority = input.value(QStringLiteral("priority")).toInt();
    fields.percent = percent;
    fields.status = status;
    fields.completed = Ics::isCompleted(status) ? Ics::nowStamp() : QString();

    task.ics = task.ics.isEmpty() ? Ics::create(fields) : Ics::rewrite(task.ics, fields);
    task.summary = fields.summary;
    task.description = fields.description;
    task.location = fields.location;
    task.url = fields.url;
    task.tags = fields.tags;
    task.due = fields.due;
    task.parentUid = fields.parentUid;
    task.priority = fields.priority;
    task.percent = percent;
    task.reminder = fields.reminder;
    task.status = fields.status;
    task.completed = fields.completed;
    task.dirty = true;
    task.deleted = false;
    return task;
}

void Session::pushDirty()
{
    if (!m_config->isConfigured() || m_busy)
        return;

    m_pendingUploads = withoutConflicts(m_store->dirtyTasks());
    if (m_pendingUploads.isEmpty())
        return;

    m_pushOnly = true;
    m_uploaded = 0;
    setBusy(true);
    pushNext();
}

// A task carried to another list is written under its new collection and
// deleted from the old one; the subtree follows, since RELATED-TO resolves by
// uid only within one collection.
void Session::moveSubtree(const QString &href, const QString &calendarHref)
{
    const LocalTask root = m_store->task(href);
    if (root.href.isEmpty())
        return;

    const QVector<LocalTask> following = m_store->descendants(href);
    // Only the root leaves a parent behind; the subtree keeps its own links.
    m_store->upsertLocal(detached(relocated(root, calendarHref)));
    for (const LocalTask &task : following)
        m_store->upsertLocal(relocated(task, calendarHref));

    dropTask(root);
    for (const LocalTask &task : following)
        dropTask(task);

    emit taskMoved(root.href, hrefIn(calendarHref, root.uid));
}

void Session::saveTask(const QVariantMap &fields)
{
    const QString target = fields.value(QStringLiteral("calendarHref")).toString();
    if (target.isEmpty()) {
        setError(tr("Choose a list for the task."));
        return;
    }

    // editedTask() leaves an existing task in the collection it already sits
    // in, so the edit lands there first and the move carries it over after.
    const LocalTask existing = m_store->task(fields.value(QStringLiteral("href")).toString());
    const bool moving = !existing.href.isEmpty()
            && Url::pathWithSlash(existing.calendarHref) != Url::pathWithSlash(target);

    m_store->upsertLocal(editedTask(fields));
    if (moving)
        moveSubtree(existing.href, target);

    reloadModels();
    pushDirty();
}

void Session::dropTask(const LocalTask &task)
{
    // A task the server never saw can go at once; the rest must be deleted
    // there first, so it is only marked here.
    if (task.etag.isEmpty())
        m_store->remove(task.href);
    else
        m_store->markDeleted(task.href);
}

void Session::deleteTask(const QString &href)
{
    const QVector<LocalTask> children = m_store->descendants(href);
    for (const LocalTask &child : children)
        dropTask(child);
    dropTask(m_store->task(href));
    reloadModels();
    pushDirty();
}

void Session::applyPercent(const QString &href, int percent)
{
    QVariantMap fields = taskFields(href);
    if (fields.isEmpty())
        return;

    // The quick toggle works the status out from the percentage, so it must
    // override the stored status the snapshot carried along.
    fields.insert(QStringLiteral("percent"), percent);
    fields.insert(QStringLiteral("status"), Ics::statusFromPercent(percent));
    saveTask(fields);
}

void Session::toggleDone(const QString &href)
{
    const QVariantMap fields = taskFields(href);
    if (fields.isEmpty())
        return;

    const bool done = fields.value(QStringLiteral("percent")).toInt() >= 100;
    applyPercent(href, done ? 0 : 100);
}

void Session::markDone(const QString &href)
{
    const QVariantMap fields = taskFields(href);
    if (fields.isEmpty())
        return;
    if (fields.value(QStringLiteral("percent")).toInt() >= 100)
        return;

    applyPercent(href, 100);
}

QString Session::renderNotes(const QString &markdown) const
{
    return Markdown::toHtml(markdown);
}

// The snapshot TaskDetailPage expects, so a reminder tap can open a task from
// its href alone. An unknown href yields an empty map.
QVariantMap Session::taskFields(const QString &href) const
{
    const LocalTask task = m_store->task(href);
    if (task.href.isEmpty())
        return QVariantMap();

    // The start date is only kept in the ICS, so it is read from there.
    const TodoFields parsed = Ics::parse(task.ics);

    QVariantMap fields;
    fields.insert(QStringLiteral("href"), task.href);
    fields.insert(QStringLiteral("calendarHref"), task.calendarHref);
    fields.insert(QStringLiteral("uid"), task.uid);
    fields.insert(QStringLiteral("parentUid"), task.parentUid);
    fields.insert(QStringLiteral("summary"), task.summary);
    fields.insert(QStringLiteral("description"), task.description);
    fields.insert(QStringLiteral("location"), task.location);
    fields.insert(QStringLiteral("url"), task.url);
    fields.insert(QStringLiteral("tags"), toVariantList(task.tags));
    fields.insert(QStringLiteral("status"), task.status);
    fields.insert(QStringLiteral("startDate"), Ics::dateFromDue(parsed.start));
    fields.insert(QStringLiteral("startTime"), Ics::timeFromDue(parsed.start));
    fields.insert(QStringLiteral("dueDate"), Ics::dateFromDue(task.due));
    fields.insert(QStringLiteral("dueTime"), Ics::timeFromDue(task.due));
    fields.insert(QStringLiteral("priority"), task.priority);
    fields.insert(QStringLiteral("percent"),
                  Ics::percentFromStatus(task.status, parsed.percent));
    fields.insert(QStringLiteral("reminder"), task.reminder);
    return fields;
}

QVariantList Session::subtasks(const QString &parentUid) const
{
    QVariantList out;
    const QVector<LocalTask> children = m_store->children(parentUid);
    for (const LocalTask &child : children) {
        // A subtask row draws a toggle, which the detail page itself has no
        // property for because it works the state out from the percentage, and
        // drops it altogether once the task is cancelled.
        QVariantMap fields = taskFields(child.href);
        fields.insert(QStringLiteral("done"), Ics::isCompleted(child.status));
        fields.insert(QStringLiteral("cancelled"), Ics::isCancelled(child.status));
        out.append(fields);
    }
    return out;
}

// How many tasks a delete would take with it, the parent included.
int Session::subtreeSize(const QString &href) const
{
    return m_store->descendants(href).size() + 1;
}
