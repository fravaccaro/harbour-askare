#include "loginflow.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

#include "url.h"

namespace {

QJsonObject objectFrom(const QByteArray &body)
{
    return QJsonDocument::fromJson(body).object();
}

QString jsonError(int status)
{
    if (status == 0)
        return QObject::tr("The server did not answer.");

    return QObject::tr("The server refused the login (%1).").arg(status);
}

// Nextcloud drops a login flow v2 token twenty minutes after it hands it out,
// and the poll endpoint answers 404 either way, so there is nothing to tell an
// expired attempt from one still waiting to be approved. Giving up when the
// server does is what turns an endless poll into something the user is told.
const int PollDeadlineMs = 20 * 60 * 1000;

}

LoginFlow::LoginFlow(QObject *parent)
    : QObject(parent)
    , m_starting(false)
    , m_active(false)
{
    m_http.setUserAgent(QStringLiteral("Askare"));
    m_timer.setInterval(2000);
    m_deadline.setSingleShot(true);
    m_deadline.setInterval(PollDeadlineMs);
    connect(&m_http, &HttpClient::finished, this, &LoginFlow::onHttpFinished);
    connect(&m_http, &HttpClient::failed, this, &LoginFlow::onHttpFailed);
    connect(&m_timer, &QTimer::timeout, this, &LoginFlow::pollOnce);
    connect(&m_deadline, &QTimer::timeout, this, &LoginFlow::onDeadline);
}

void LoginFlow::start(const QString &serverUrl, bool ignoreSsl)
{
    cancel();
    const QString normalized = Url::normalizeServer(serverUrl);
    if (normalized.isEmpty()) {
        emit failed(tr("Enter a server address."));
        return;
    }

    m_active = true;
    m_starting = true;
    m_http.setIgnoreSslErrors(ignoreSsl);
    const QUrl url(normalized + QStringLiteral("/index.php/login/v2"));
    HttpClient::Headers headers;
    headers.append(qMakePair(QByteArray("Content-Type"),
                             QByteArray("application/x-www-form-urlencoded")));
    m_http.send("POST", url, QByteArray(), headers);
}

void LoginFlow::cancel()
{
    m_timer.stop();
    m_deadline.stop();
    m_active = false;
    m_starting = false;
    m_token.clear();
    m_pollUrl.clear();
}

void LoginFlow::onHttpFinished(int status, const QByteArray &body, const QString &,
                               const QUrl &)
{
    if (!m_active)
        return;

    if (m_starting)
        handleStart(status, body);
    else
        handlePoll(status, body);
}

void LoginFlow::onHttpFailed(const QString &error)
{
    if (!m_active)
        return;

    cancel();
    emit failed(error);
}

void LoginFlow::handleStart(int status, const QByteArray &body)
{
    if (status != 200) {
        cancel();
        emit failed(jsonError(status));
        return;
    }

    const QJsonObject object = objectFrom(body);
    const QJsonObject poll = object.value(QStringLiteral("poll")).toObject();
    m_token = poll.value(QStringLiteral("token")).toString();
    m_pollUrl = QUrl(poll.value(QStringLiteral("endpoint")).toString());
    const QString login = object.value(QStringLiteral("login")).toString();
    if (m_token.isEmpty() || !m_pollUrl.isValid() || login.isEmpty()) {
        cancel();
        emit failed(tr("The server did not return a login page."));
        return;
    }

    m_starting = false;
    emit loginUrlReady(login);
    m_timer.start();
    m_deadline.start();
}

void LoginFlow::onDeadline()
{
    cancel();
    emit failed(tr("The sign-in attempt took too long. Start again."));
}

void LoginFlow::handlePoll(int status, const QByteArray &body)
{
    if (status == 404)
        return;

    if (status != 200) {
        cancel();
        emit failed(jsonError(status));
        return;
    }

    const QJsonObject object = objectFrom(body);
    const QString server = object.value(QStringLiteral("server")).toString();
    const QString loginName = object.value(QStringLiteral("loginName")).toString();
    const QString password = object.value(QStringLiteral("appPassword")).toString();
    cancel();
    if (loginName.isEmpty() || password.isEmpty()) {
        emit failed(tr("The server did not return a password."));
        return;
    }

    emit finished(Url::normalizeServer(server), loginName, password);
}

void LoginFlow::pollOnce()
{
    if (!m_active || m_token.isEmpty())
        return;

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("token"), m_token);
    HttpClient::Headers headers;
    headers.append(qMakePair(QByteArray("Content-Type"),
                             QByteArray("application/x-www-form-urlencoded")));
    m_http.send("POST", m_pollUrl, query.toString(QUrl::FullyEncoded).toUtf8(), headers);
}
