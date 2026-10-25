#ifndef ASKARE_HTTPCLIENT_H
#define ASKARE_HTTPCLIENT_H

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPair>
#include <QString>
#include <QUrl>
#include <QVector>

class HttpClient : public QObject
{
    Q_OBJECT

public:
    typedef QVector<QPair<QByteArray, QByteArray> > Headers;

    explicit HttpClient(QObject *parent = nullptr);

    void setUserAgent(const QString &agent);
    void setBasicAuth(const QString &user, const QString &password);
    void setIgnoreSslErrors(bool ignore);

    void send(const QByteArray &method, const QUrl &url,
              const QByteArray &body = QByteArray(),
              const Headers &headers = Headers());

signals:
    void finished(int status, const QByteArray &body, const QString &etag,
                  const QUrl &url);
    void failed(const QString &error);

private:
    void onFinished();
    void applyAuth(QNetworkRequest *request) const;

    QNetworkAccessManager m_manager;
    QString m_userAgent;
    QString m_user;
    QString m_password;
    bool m_ignoreSsl;
};

#endif
