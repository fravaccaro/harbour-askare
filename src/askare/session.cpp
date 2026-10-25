#include "session.h"

#include "calendarmodel.h"
#include "ics.h"
#include "markdown.h"
#include "taskmodel.h"
#include "url.h"

namespace {

void ensureIdentity(LocalTask *task, const QString &calendarHref)
{
    if (!task->href.isEmpty())
        return;

    task->uid = Ics::newUid();
    task->href = Url::pathWithSlash(calendarHref) + task->uid + QStringLiteral(".ics");
    task->calendarHref = Url::pathWithSlash(calendarHref);
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
    connect(m_config, &Config::readyChanged, this, &Session::readyChanged);
    connect(m_config, &Config::configuredChanged, this, &Session::onConfiguredChanged);
    connect(m_config, &Config::secretsErrorChanged, this, &Session::secretsErrorChanged);
    connect(m_config, &Config::serverUrlChanged, this, &Session::serverUrlChanged);
    connect(m_config, &Config::ignoreSslErrorsChanged, this, &Session::ignoreSslErrorsChanged);
    connect(m_config, &Config::syncOnStartChanged, this, &Session::syncOnStartChanged);
    connect(m_config, &Config::syncAfterPushChanged, this, &Session::syncAfterPushChanged);
    connect(m_config, &Config::syncIntervalChanged, this, &Session::syncIntervalChanged);
    connect(m_config, &Config::syncIntervalChanged, this, &Session::applySyncTimer);
    connect(m_config, &Config::readyChanged, this, &Session::syncIfStartupWanted);
    m_store->open();
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

void Session::onLoginFailed(const QString &error)
{
    setBusy(false);
    setError(error);
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
        m_pendingUploads = m_store->dirtyTasks();
        pushNext();
        return;
    }

    const CalendarInfo info = m_pendingCalendars.takeFirst();
    m_currentCalendarHref = Url::pathWithSlash(info.href);
    m_dav.fetchTodos(m_currentCalendarHref);
}

void Session::onTodos(const QVector<RemoteTask> &tasks)
{
    m_store->mergeRemote(m_currentCalendarHref, tasks);
    fetchNextCalendar();
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
    setBusy(false);
    reloadModels();
    if (m_syncTimer.isActive())
        m_syncTimer.start();
    pushDirty();
    if (pullAfterPush && !m_busy)
        QTimer::singleShot(0, this, &Session::sync);
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

LocalTask Session::editedTask(const QString &href, const QString &calendarHref,
                              const QString &summary, const QString &description,
                              const QString &dueDate, const QString &dueTime,
                              int priority, int percent, int reminder) const
{
    LocalTask task = m_store->task(href);
    ensureIdentity(&task, calendarHref);
    TodoFields fields = Ics::parse(task.ics);
    fields.uid = task.uid.isEmpty() ? fields.uid : task.uid;
    fields.summary = summary;
    fields.description = description;
    fields.due = dueDate.isEmpty() ? QString() : Ics::dueFromDateTime(dueDate, dueTime);
    // A reminder is relative to the due time, so it needs one.
    fields.reminder = fields.due.isEmpty() ? -1 : reminder;
    fields.priority = priority;
    fields.percent = percent;
    fields.status = Ics::statusFromPercent(percent);
    fields.completed = percent >= 100 ? Ics::nowStamp() : QString();
    task.ics = task.ics.isEmpty() ? Ics::create(fields) : Ics::rewrite(task.ics, fields);
    task.summary = summary;
    task.description = description;
    task.due = fields.due;
    task.priority = priority;
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

    m_pendingUploads = m_store->dirtyTasks();
    if (m_pendingUploads.isEmpty())
        return;

    m_pushOnly = true;
    m_uploaded = 0;
    setBusy(true);
    pushNext();
}

void Session::saveTask(const QString &href, const QString &calendarHref,
                       const QString &summary, const QString &description,
                       const QString &dueDate, const QString &dueTime,
                       int priority, int percent, int reminder)
{
    if (calendarHref.isEmpty()) {
        setError(tr("Choose a list for the task."));
        return;
    }

    m_store->upsertLocal(editedTask(href, calendarHref, summary, description,
                                    dueDate, dueTime, priority, percent, reminder));
    reloadModels();
    pushDirty();
}

void Session::deleteTask(const QString &href)
{
    const LocalTask task = m_store->task(href);
    if (task.etag.isEmpty())
        m_store->remove(href);
    else
        m_store->markDeleted(href);
    reloadModels();
    pushDirty();
}

void Session::toggleDone(const QString &href)
{
    const LocalTask task = m_store->task(href);
    if (task.href.isEmpty())
        return;

    const int percent = Ics::isCompleted(task.status) ? 0 : 100;
    saveTask(task.href, task.calendarHref, task.summary, task.description,
             Ics::dateFromDue(task.due), Ics::timeFromDue(task.due), task.priority,
             percent, task.reminder);
}

QString Session::renderNotes(const QString &markdown) const
{
    return Markdown::toHtml(markdown);
}
