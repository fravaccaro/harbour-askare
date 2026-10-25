#include "config.h"

#include <QDir>
#include <QStandardPaths>

#include "url.h"

namespace {

QString settingsFilePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/settings.conf");
}

}

Config::Config(QObject *parent)
    : QObject(parent)
    , m_settings(settingsFilePath(), QSettings::IniFormat)
    , m_ignoreSslErrors(false)
    , m_ready(false)
{
    m_serverUrl = m_settings.value(QStringLiteral("server/url")).toString();
    m_username = m_settings.value(QStringLiteral("server/username")).toString();
    m_ignoreSslErrors = m_settings.value(QStringLiteral("server/ignoreSslErrors"), false).toBool();
    connect(&m_secrets, &SecretsStore::loaded, this, &Config::applyStoredPassword);
    connect(&m_secrets, &SecretsStore::failed, this, [this](const QString &error) {
        m_secretsError = error;
        emit secretsErrorChanged();
    });
    m_secrets.load();
}

bool Config::isConfigured() const
{
    return !m_serverUrl.isEmpty() && !m_username.isEmpty() && !m_password.isEmpty();
}

void Config::reloadSecrets()
{
    if (!m_secretsError.isEmpty()) {
        m_secretsError.clear();
        emit secretsErrorChanged();
    }
    m_secrets.load();
}

void Config::applyStoredPassword(const QString &password)
{
    const bool wasConfigured = isConfigured();
    m_password = password;
    emit passwordChanged();
    if (wasConfigured != isConfigured())
        emit configuredChanged();
    m_ready = true;
    emit readyChanged();
}

void Config::store(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
    m_settings.sync();
}

void Config::setServerUrl(const QString &url)
{
    const QString normalized = Url::normalizeServer(url);
    if (normalized == m_serverUrl)
        return;

    const bool wasConfigured = isConfigured();
    m_serverUrl = normalized;
    store(QStringLiteral("server/url"), m_serverUrl);
    emit serverUrlChanged();
    if (wasConfigured != isConfigured())
        emit configuredChanged();
}

void Config::setUsername(const QString &username)
{
    if (username == m_username)
        return;

    const bool wasConfigured = isConfigured();
    m_username = username;
    store(QStringLiteral("server/username"), m_username);
    emit usernameChanged();
    if (wasConfigured != isConfigured())
        emit configuredChanged();
}

void Config::setPassword(const QString &password)
{
    if (password == m_password)
        return;

    const bool wasConfigured = isConfigured();
    m_password = password;
    if (m_password.isEmpty())
        m_secrets.remove();
    else
        m_secrets.save(m_password);
    emit passwordChanged();
    if (wasConfigured != isConfigured())
        emit configuredChanged();
}

void Config::setIgnoreSslErrors(bool ignore)
{
    if (ignore == m_ignoreSslErrors)
        return;

    m_ignoreSslErrors = ignore;
    store(QStringLiteral("server/ignoreSslErrors"), m_ignoreSslErrors);
    emit ignoreSslErrorsChanged();
}

void Config::clearCredentials()
{
    setPassword(QString());
    setUsername(QString());
}

QString Config::normalizeServerUrl(const QString &input)
{
    return Url::normalizeServer(input);
}
