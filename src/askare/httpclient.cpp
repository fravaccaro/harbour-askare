#include "httpclient.h"

#include <QBuffer>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslError>

namespace {

QString networkError(QNetworkReply *reply)
{
    switch (reply->error()) {
    case QNetworkReply::HostNotFoundError:
        return QObject::tr("The host was not found.");
    case QNetworkReply::ConnectionRefusedError:
        return QObject::tr("The server refused the connection.");
    case QNetworkReply::SslHandshakeFailedError:
        return QObject::tr("The certificate of the server was not accepted.");
    case QNetworkReply::TimeoutError:
    case QNetworkReply::TemporaryNetworkFailureError:
    case QNetworkReply::NetworkSessionFailedError:
        return QObject::tr("The network did not answer.");
    default:
        return reply->errorString();
    }
}

}

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , m_userAgent(QStringLiteral("Askare"))
    , m_ignoreSsl(false)
{
}

void HttpClient::setUserAgent(const QString &agent)
{
    m_userAgent = agent;
}

void HttpClient::setBasicAuth(const QString &user, const QString &password)
{
    m_user = user;
    m_password = password;
}

void HttpClient::setIgnoreSslErrors(bool ignore)
{
    m_ignoreSsl = ignore;
}

void HttpClient::applyAuth(QNetworkRequest *request) const
{
    if (m_user.isEmpty())
        return;

    const QByteArray token = (m_user + QLatin1Char(':') + m_password).toUtf8().toBase64();
    request->setRawHeader("Authorization", "Basic " + token);
}

void HttpClient::send(const QByteArray &method, const QUrl &url,
                      const QByteArray &body, const Headers &headers)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    applyAuth(&request);
    for (int i = 0; i < headers.size(); ++i)
        request.setRawHeader(headers.at(i).first, headers.at(i).second);

    QBuffer *buffer = new QBuffer;
    buffer->setData(body);
    buffer->open(QIODevice::ReadOnly);
    QNetworkReply *reply = m_manager.sendCustomRequest(request, method, buffer);
    buffer->setParent(reply);
    reply->setProperty("requestUrl", url);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onFinished);
    if (m_ignoreSsl) {
        connect(reply, &QNetworkReply::sslErrors, this, [reply](const QList<QSslError> &) {
            reply->ignoreSslErrors();
        });
    }
}

void HttpClient::onFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    reply->deleteLater();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError && status == 0) {
        emit failed(networkError(reply));
        return;
    }

    emit finished(status, reply->readAll(),
                  QString::fromLatin1(reply->rawHeader("ETag")),
                  reply->property("requestUrl").toUrl());
}
