#ifndef ASKARE_SESSION_H
#define ASKARE_SESSION_H

#include <QObject>
#include <QPair>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include "config.h"
#include "davclient.h"
#include "loginflow.h"
#include "store.h"
#include "types.h"

class CalendarModel;
class SmartListModel;
class TaskModel;

class Session : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
    Q_PROPERTY(bool configured READ isConfigured NOTIFY configuredChanged)
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QString secretsError READ secretsError NOTIFY secretsErrorChanged)
    Q_PROPERTY(QString loginUrl READ loginUrl NOTIFY loginUrlChanged)
    Q_PROPERTY(QString serverUrl READ serverUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
    Q_PROPERTY(bool ignoreSslErrors READ ignoreSslErrors WRITE setIgnoreSslErrors
               NOTIFY ignoreSslErrorsChanged)
    Q_PROPERTY(bool syncOnStart READ syncOnStart WRITE setSyncOnStart NOTIFY syncOnStartChanged)
    Q_PROPERTY(int syncInterval READ syncInterval WRITE setSyncInterval
               NOTIFY syncIntervalChanged)
    Q_PROPERTY(bool syncAfterPush READ syncAfterPush WRITE setSyncAfterPush
               NOTIFY syncAfterPushChanged)
    Q_PROPERTY(bool showCompletedTasks READ showCompletedTasks WRITE setShowCompletedTasks
               NOTIFY showCompletedTasksChanged)
    Q_PROPERTY(bool showCancelledTasks READ showCancelledTasks WRITE setShowCancelledTasks
               NOTIFY showCancelledTasksChanged)
    Q_PROPERTY(int openCount READ openCount NOTIFY coverChanged)
    Q_PROPERTY(int currentCount READ currentCount NOTIFY coverChanged)
    Q_PROPERTY(int todayCount READ todayCount NOTIFY coverChanged)
    Q_PROPERTY(int importantCount READ importantCount NOTIFY coverChanged)
    Q_PROPERTY(int completedCount READ completedCount NOTIFY coverChanged)
    Q_PROPERTY(QString nextDueSummary READ nextDueSummary NOTIFY coverChanged)
    // One entry per tag in use, as { name, count }, for the lists page.
    Q_PROPERTY(QVariantList tags READ tags NOTIFY coverChanged)
    Q_PROPERTY(int conflictCount READ conflictCount NOTIFY conflictsChanged)

public:
    explicit Session(Config *config, Store *store, CalendarModel *calendars,
                     TaskModel *tasks, SmartListModel *smartLists,
                     QObject *parent = nullptr);

    bool isReady() const;
    bool isConfigured() const;
    bool isBusy() const { return m_busy; }
    QString errorMessage() const { return m_error; }
    QString secretsError() const;
    QString loginUrl() const { return m_loginUrl; }
    QString serverUrl() const;
    QString username() const;
    bool ignoreSslErrors() const;
    bool syncOnStart() const;
    int syncInterval() const;
    bool syncAfterPush() const;
    bool showCompletedTasks() const;
    bool showCancelledTasks() const;
    int openCount() const;
    int currentCount() const;
    int todayCount() const;
    int importantCount() const;
    int completedCount() const;
    QString nextDueSummary() const;
    QVariantList tags() const;
    int conflictCount() const { return m_conflicts.size(); }

    Q_INVOKABLE void startLogin(const QString &serverUrl);
    Q_INVOKABLE void cancelLogin();
    Q_INVOKABLE void logout();
    Q_INVOKABLE void reloadSecrets();
    Q_INVOKABLE void sync();
    // Takes the same keys taskFields() hands out, so the two stay symmetric.
    Q_INVOKABLE void saveTask(const QVariantMap &fields);
    Q_INVOKABLE void deleteTask(const QString &href);
    Q_INVOKABLE void toggleDone(const QString &href);
    // Completes a task without flipping it open again.
    Q_INVOKABLE void markDone(const QString &href);
    Q_INVOKABLE QString renderNotes(const QString &markdown) const;
    Q_INVOKABLE QVariantMap taskFields(const QString &href) const;
    // The task another app's share makes, keyed as taskFields() is, so the
    // editor is opened with it the way an existing task is. Neither writes
    // anything: the task is the editor's to save, as one typed by hand is. An
    // empty answer means the share held nothing worth writing down.
    Q_INVOKABLE QVariantMap sharedTextFields(const QString &text) const;
    Q_INVOKABLE QVariantMap sharedLinkFields(const QString &url,
                                             const QString &linkTitle) const;
    Q_INVOKABLE QVariantList subtasks(const QString &parentUid) const;
    Q_INVOKABLE int subtreeSize(const QString &href) const;
    // One entry per task waiting to be settled, as { href, summary, differences },
    // where differences names only the fields the two versions disagree on.
    Q_INVOKABLE QVariantList conflicts() const;
    Q_INVOKABLE void resolveKeepMine(const QString &href);
    Q_INVOKABLE void resolveKeepTheirs(const QString &href);
    // Files the filters as a view of its own, under the name given.
    Q_INVOKABLE int saveSmartList(const QString &name, const QVariantMap &filters);
    // Deleting the set now on show leaves the task list on All.
    Q_INVOKABLE void deleteSmartList(int id);

signals:
    void readyChanged();
    void configuredChanged();
    void busyChanged();
    void errorMessageChanged();
    void secretsErrorChanged();
    void loginUrlChanged();
    void serverUrlChanged();
    void usernameChanged();
    void ignoreSslErrorsChanged();
    void syncOnStartChanged();
    void syncIntervalChanged();
    void syncAfterPushChanged();
    void showCompletedTasksChanged();
    void showCancelledTasksChanged();
    void coverChanged();
    void tasksChanged();
    void conflictsChanged();
    void loginUrlReady(const QString &url);
    void loginSucceeded();
    void loginFailed(const QString &error);
    // A task carried to another list keeps its uid but not its href, so a page
    // holding the old one can follow it.
    void taskMoved(const QString &oldHref, const QString &newHref);

public slots:
    void setIgnoreSslErrors(bool ignore);
    void setSyncOnStart(bool enabled);
    void setSyncInterval(int minutes);
    void setSyncAfterPush(bool enabled);
    void setShowCompletedTasks(bool show);
    void setShowCancelledTasks(bool show);

private:
    void setBusy(bool busy);
    void setError(const QString &error);
    void applyCredentials();
    void applySyncTimer();
    void applyShowCompleted();
    void applyShowCancelled();
    void syncIfStartupWanted();
    void reloadModels();
    void dropTask(const LocalTask &task);
    void moveSubtree(const QString &href, const QString &calendarHref);
    void fetchNextCalendar();
    void pushNext();
    void finishSync();
    void pushDirty();
    void applyPercent(const QString &href, int percent);
    LocalTask editedTask(const QVariantMap &fields) const;
    void onLoginUrl(const QString &url);
    void onLoggedIn(const QString &server, const QString &user,
                    const QString &password);
    void onLoginFailed(const QString &error);
    void onPrincipal(const QString &href);
    void onHome(const QString &href);
    void onCalendars(const QVector<CalendarInfo> &calendars);
    void onTodos(const QVector<RemoteTask> &tasks);
    void onUploaded(const QString &href, const QString &etag);
    void onRemoved(const QString &href);
    void onDavFailed(const QString &error);
    void onConflicted(const QString &href);
    void onConfiguredChanged();
    void noteConflict(const RemoteTask &remote);
    // Settles the tasks the server refused and then did not list, the pull
    // being the only thing that can say a refused task is gone rather than
    // merely changed.
    void settleVanished(const QVector<RemoteTask> &seen);
    void restartVanished(const LocalTask &mine);
    int indexOfConflict(const QString &href) const;
    bool takeConflict(const QString &href, QPair<LocalTask, RemoteTask> *pair);
    QVector<LocalTask> withoutConflicts(const QVector<LocalTask> &tasks) const;
    QVariantMap conflictRow(const QPair<LocalTask, RemoteTask> &pair) const;
    QVariantList differencesBetween(const TodoFields &mine, const TodoFields &theirs) const;
    QString taskName(const QString &uid) const;

    Config *m_config;
    Store *m_store;
    CalendarModel *m_calendars;
    TaskModel *m_tasks;
    SmartListModel *m_smartLists;
    LoginFlow m_login;
    DavClient m_dav;
    QTimer m_syncTimer;
    QVector<CalendarInfo> m_pendingCalendars;
    QVector<LocalTask> m_pendingUploads;
    // The task the write now on the wire was built from. The answer is taken
    // against this rather than against the row, which the user may have
    // edited again while that answer was on its way.
    LocalTask m_inFlight;
    // Tasks changed here and on the server at once, each the edit made here
    // beside the version the server holds. Kept only while the app runs: one
    // closed before the question is answered meets the task on the next sync.
    QVector<QPair<LocalTask, RemoteTask> > m_conflicts;
    // Tasks the server refused for their version, held back until a pull says
    // what to make of them, so a refusal is never answered by sending again.
    QSet<QString> m_refused;
    QString m_currentCalendarHref;
    QString m_loginUrl;
    QString m_error;
    bool m_busy;
    // True while a run only uploads, so it can be followed by a full sync.
    bool m_pushOnly;
    // A refused write earns the run a pull, which is what finds the conflict.
    bool m_pullAfterConflict;
    // An edit made while a run was on the wire, owed a push once it ends.
    bool m_pushDeferred;
    int m_uploaded;
    bool m_syncedOnStart;
};

#endif
