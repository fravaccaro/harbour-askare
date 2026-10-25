#ifndef ASKARE_SESSION_H
#define ASKARE_SESSION_H

#include <QObject>
#include <QString>
#include <QTimer>
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
    int openCount() const;
    int currentCount() const;
    int completedCount() const;
    QString nextDueSummary() const;

    Q_INVOKABLE void startLogin(const QString &serverUrl);
    Q_INVOKABLE void cancelLogin();
    Q_INVOKABLE void logout();
    Q_INVOKABLE void reloadSecrets();
    Q_INVOKABLE void sync();
    Q_INVOKABLE void saveTask(const QString &href, const QString &calendarHref,
                              const QString &summary, const QString &description,
                              const QString &dueDate, int priority, int percent);
    Q_INVOKABLE void deleteTask(const QString &href);
    Q_INVOKABLE void toggleDone(const QString &href);
    Q_INVOKABLE QString renderNotes(const QString &markdown) const;

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
    void coverChanged();
    void loginUrlReady(const QString &url);
    void loginSucceeded();
    void loginFailed(const QString &error);

public slots:
    void setIgnoreSslErrors(bool ignore);
    void setSyncOnStart(bool enabled);
    void setSyncInterval(int minutes);
    void setSyncAfterPush(bool enabled);

private:
    void setBusy(bool busy);
    void setError(const QString &error);
    void applyCredentials();
    void applySyncTimer();
    void syncIfStartupWanted();
    void reloadModels();
    void fetchNextCalendar();
    void pushNext();
    void finishSync();
    void pushDirty();
    LocalTask editedTask(const QString &href, const QString &calendarHref,
                         const QString &summary, const QString &description,
                         const QString &dueDate, int priority, int percent) const;
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
