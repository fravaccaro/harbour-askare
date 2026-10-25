#ifndef ASKARE_CONFIG_H
#define ASKARE_CONFIG_H

#include <QObject>
#include <QSettings>
#include <QString>

#include "secretsstore.h"

class Config : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(bool ignoreSslErrors READ ignoreSslErrors WRITE setIgnoreSslErrors
               NOTIFY ignoreSslErrorsChanged)
    Q_PROPERTY(bool syncOnStart READ syncOnStart WRITE setSyncOnStart NOTIFY syncOnStartChanged)
    Q_PROPERTY(int syncInterval READ syncInterval WRITE setSyncInterval
               NOTIFY syncIntervalChanged)
    Q_PROPERTY(bool syncAfterPush READ syncAfterPush WRITE setSyncAfterPush
               NOTIFY syncAfterPushChanged)
    Q_PROPERTY(bool configured READ isConfigured NOTIFY configuredChanged)
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
    Q_PROPERTY(QString secretsError READ secretsError NOTIFY secretsErrorChanged)

public:
    explicit Config(QObject *parent = nullptr);

    QString serverUrl() const { return m_serverUrl; }
    void setServerUrl(const QString &url);

    QString username() const { return m_username; }
    void setUsername(const QString &username);

    QString password() const { return m_password; }
    void setPassword(const QString &password);

    bool ignoreSslErrors() const { return m_ignoreSslErrors; }
    void setIgnoreSslErrors(bool ignore);

    bool syncOnStart() const { return m_syncOnStart; }
    void setSyncOnStart(bool enabled);

    // Minutes between automatic syncs; 0 turns the timer off.
    int syncInterval() const { return m_syncInterval; }
    void setSyncInterval(int minutes);

    bool syncAfterPush() const { return m_syncAfterPush; }
    void setSyncAfterPush(bool enabled);

    bool isConfigured() const;
    bool isReady() const { return m_ready; }
    QString secretsError() const { return m_secretsError; }

    Q_INVOKABLE void clearCredentials();
    Q_INVOKABLE void reloadSecrets();
    Q_INVOKABLE static QString normalizeServerUrl(const QString &input);

signals:
    void serverUrlChanged();
    void usernameChanged();
    void passwordChanged();
    void ignoreSslErrorsChanged();
    void syncOnStartChanged();
    void syncIntervalChanged();
    void syncAfterPushChanged();
    void configuredChanged();
    void readyChanged();
    void secretsErrorChanged();

private:
    void store(const QString &key, const QVariant &value);
    void applyStoredPassword(const QString &password);

    QSettings m_settings;
    SecretsStore m_secrets;
    QString m_serverUrl;
    QString m_username;
    QString m_password;
    QString m_secretsError;
    bool m_ignoreSslErrors;
    bool m_syncOnStart;
    int m_syncInterval;
    bool m_syncAfterPush;
    bool m_ready;
};

#endif
