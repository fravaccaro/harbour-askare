#ifndef ASKARE_SESSION_H
#define ASKARE_SESSION_H

#include <QObject>
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
    Q_PROPERTY(bool ignoreSslErrors READ ignoreSslErrors WRITE setIgnoreSslErrors
               NOTIFY ignoreSslErrorsChanged)
    Q_PROPERTY(bool syncOnStart READ syncOnStart WRITE setSyncOnStart NOTIFY syncOnStartChanged)
    Q_PROPERTY(int syncInterval READ syncInterval WRITE setSyncInterval
               NOTIFY syncIntervalChanged)
    Q_PROPERTY(bool syncAfterPush READ syncAfterPush WRITE setSyncAfterPush
               NOTIFY syncAfterPushChanged)
    Q_PROPERTY(bool showCompletedTasks READ showCompletedTasks WRITE setShowCompletedTasks
               NOTIFY showCompletedTasksChanged)
    Q_PROPERTY(int openCount READ openCount NOTIFY coverChanged)
    Q_PROPERTY(int currentCount READ currentCount NOTIFY coverChanged)
    Q_PROPERTY(int completedCount READ completedCount NOTIFY coverChanged)
    Q_PROPERTY(QString nextDueSummary READ nextDueSummary NOTIFY coverChanged)

public:
    explicit Session(Config *config, Store *store, CalendarModel *calendars,
                     TaskModel *tasks, QObject *parent = nullptr);

    bool isReady() const;
    bool isConfigured() const;
    bool isBusy() const { return m_busy; }
    QString errorMessage() const { return m_error; }
    QString secretsError() const;
    QString loginUrl() const { return m_loginUrl; }
    QString serverUrl() const;
    bool ignoreSslErrors() const;
    bool syncOnStart() const;
    int syncInterval() const;
    bool syncAfterPush() const;
    bool showCompletedTasks() const;
    int openCount() const;
    int currentCount() const;
    int completedCount() const;
    QString nextDueSummary() const;

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
    Q_INVOKABLE QVariantList subtasks(const QString &parentUid) const;
    Q_INVOKABLE int subtreeSize(const QString &href) const;

signals:
    void readyChanged();
    void configuredChanged();
    void busyChanged();
    void errorMessageChanged();
    void secretsErrorChanged();
    void loginUrlChanged();
    void serverUrlChanged();
    void ignoreSslErrorsChanged();
    void syncOnStartChanged();
    void syncIntervalChanged();
    void syncAfterPushChanged();
    void showCompletedTasksChanged();
    void coverChanged();
    void tasksChanged();
    void loginUrlReady(const QString &url);
    void loginSucceeded();
    void loginFailed(const QString &error);

public slots:
    void setIgnoreSslErrors(bool ignore);
    void setSyncOnStart(bool enabled);
    void setSyncInterval(int minutes);
    void setSyncAfterPush(bool enabled);
    void setShowCompletedTasks(bool show);

private:
    void setBusy(bool busy);
    void setError(const QString &error);
    void applyCredentials();
    void applySyncTimer();
    void applyShowCompleted();
    void syncIfStartupWanted();
    void reloadModels();
    void dropTask(const LocalTask &task);
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
    void onConfiguredChanged();

    Config *m_config;
    Store *m_store;
    CalendarModel *m_calendars;
    TaskModel *m_tasks;
    LoginFlow m_login;
    DavClient m_dav;
    QTimer m_syncTimer;
    QVector<CalendarInfo> m_pendingCalendars;
    QVector<LocalTask> m_pendingUploads;
    QString m_currentCalendarHref;
    QString m_loginUrl;
    QString m_error;
    bool m_busy;
    // True while a run only uploads, so it can be followed by a full sync.
    bool m_pushOnly;
    int m_uploaded;
    bool m_syncedOnStart;
};

#endif
