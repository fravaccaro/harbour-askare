#include "config.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QStringList>

#include "url.h"

namespace {

QString settingsFilePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/settings.conf");
}

// A filter set is written as one JSON line, which an ini file carries whole
// where a nested QVariantMap would arrive back as an unreadable blob.
QString filtersToText(const QVariantMap &filters)
{
    const QJsonDocument document(QJsonObject::fromVariantMap(filters));
    return QString::fromUtf8(document.toJson(QJsonDocument::Compact));
}

QVariantMap filtersFromText(const QString &text)
{
    return QJsonDocument::fromJson(text.toUtf8()).object().toVariantMap();
}

QString smartListGroup(int id)
{
    return QStringLiteral("smartLists/") + QString::number(id);
}

}

Config::Config(QObject *parent)
    : QObject(parent)
    , m_settings(settingsFilePath(), QSettings::IniFormat)
    , m_ignoreSslErrors(false)
    , m_syncOnStart(true)
    , m_syncInterval(0)
    , m_syncAfterPush(true)
    , m_showCompleted(true)
    , m_showCancelled(false)
    , m_ready(false)
{
    m_serverUrl = m_settings.value(QStringLiteral("server/url")).toString();
    m_username = m_settings.value(QStringLiteral("server/username")).toString();
    m_ignoreSslErrors = m_settings.value(QStringLiteral("server/ignoreSslErrors"), false).toBool();
    m_syncOnStart = m_settings.value(QStringLiteral("sync/onStart"), true).toBool();
    m_syncInterval = m_settings.value(QStringLiteral("sync/intervalMinutes"), 0).toInt();
    m_syncAfterPush = m_settings.value(QStringLiteral("sync/afterPush"), true).toBool();
    m_showCompleted = m_settings.value(QStringLiteral("lists/showCompleted"), true).toBool();
    m_showCancelled = m_settings.value(QStringLiteral("lists/showCancelled"), false).toBool();
    loadSmartLists();
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

void Config::setSyncOnStart(bool enabled)
{
    if (enabled == m_syncOnStart)
        return;

    m_syncOnStart = enabled;
    store(QStringLiteral("sync/onStart"), m_syncOnStart);
    emit syncOnStartChanged();
}

void Config::setSyncInterval(int minutes)
{
    const int safe = minutes < 0 ? 0 : minutes;
    if (safe == m_syncInterval)
        return;

    m_syncInterval = safe;
    store(QStringLiteral("sync/intervalMinutes"), m_syncInterval);
    emit syncIntervalChanged();
}

void Config::setSyncAfterPush(bool enabled)
{
    if (enabled == m_syncAfterPush)
        return;

    m_syncAfterPush = enabled;
    store(QStringLiteral("sync/afterPush"), m_syncAfterPush);
    emit syncAfterPushChanged();
}

void Config::setShowCompleted(bool show)
{
    if (show == m_showCompleted)
        return;

    m_showCompleted = show;
    store(QStringLiteral("lists/showCompleted"), m_showCompleted);
    emit showCompletedChanged();
}

void Config::setShowCancelled(bool show)
{
    if (show == m_showCancelled)
        return;

    m_showCancelled = show;
    store(QStringLiteral("lists/showCancelled"), m_showCancelled);
    emit showCancelledChanged();
}

void Config::loadSmartLists()
{
    m_smartLists.clear();
    const QStringList ids = m_settings.value(QStringLiteral("smartLists/ids")).toStringList();
    for (int i = 0; i < ids.size(); ++i) {
        const QString group = smartListGroup(ids.at(i).toInt()) + QLatin1Char('/');
        QVariantMap entry;
        entry.insert(QStringLiteral("id"), ids.at(i).toInt());
        entry.insert(QStringLiteral("name"),
                     m_settings.value(group + QStringLiteral("name")).toString());
        entry.insert(QStringLiteral("filters"),
                     filtersFromText(m_settings.value(group + QStringLiteral("filters")).toString()));
        m_smartLists.append(entry);
    }
}

void Config::storeSmartLists()
{
    QStringList ids;
    for (int i = 0; i < m_smartLists.size(); ++i) {
        const QVariantMap entry = m_smartLists.at(i).toMap();
        const int id = entry.value(QStringLiteral("id")).toInt();
        const QString group = smartListGroup(id) + QLatin1Char('/');
        ids.append(QString::number(id));
        m_settings.setValue(group + QStringLiteral("name"),
                            entry.value(QStringLiteral("name")).toString());
        m_settings.setValue(group + QStringLiteral("filters"),
                            filtersToText(entry.value(QStringLiteral("filters")).toMap()));
    }
    m_settings.setValue(QStringLiteral("smartLists/ids"), ids);
    m_settings.sync();
}

int Config::indexOfSmartList(int id) const
{
    for (int i = 0; i < m_smartLists.size(); ++i) {
        if (m_smartLists.at(i).toMap().value(QStringLiteral("id")).toInt() == id)
            return i;
    }
    return -1;
}

// Ids are never reused, so a page still holding a deleted one finds nothing
// rather than the set that took its place.
int Config::unusedSmartListId() const
{
    int highest = 0;
    for (int i = 0; i < m_smartLists.size(); ++i)
        highest = qMax(highest, m_smartLists.at(i).toMap().value(QStringLiteral("id")).toInt());
    return highest + 1;
}

int Config::addSmartList(const QString &name, const QVariantMap &filters)
{
    const int id = unusedSmartListId();
    QVariantMap entry;
    entry.insert(QStringLiteral("id"), id);
    entry.insert(QStringLiteral("name"), name);
    entry.insert(QStringLiteral("filters"), filters);
    m_smartLists.append(entry);
    storeSmartLists();
    emit smartListsChanged();
    return id;
}

void Config::removeSmartList(int id)
{
    const int index = indexOfSmartList(id);
    if (index < 0)
        return;

    m_smartLists.removeAt(index);
    // The group goes as well, or its name and filters would be read back the
    // next time an id climbed to the same number.
    m_settings.remove(smartListGroup(id));
    storeSmartLists();
    emit smartListsChanged();
}

QVariantMap Config::smartListFilters(int id) const
{
    const int index = indexOfSmartList(id);
    if (index < 0)
        return QVariantMap();
    return m_smartLists.at(index).toMap().value(QStringLiteral("filters")).toMap();
}

QString Config::smartListName(int id) const
{
    const int index = indexOfSmartList(id);
    if (index < 0)
        return QString();
    return m_smartLists.at(index).toMap().value(QStringLiteral("name")).toString();
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
