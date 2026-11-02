#ifndef ASKARE_LOGINFLOW_H
#define ASKARE_LOGINFLOW_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>

#include "httpclient.h"

class LoginFlow : public QObject
{
    Q_OBJECT

public:
    explicit LoginFlow(QObject *parent = nullptr);

    void start(const QString &serverUrl, bool ignoreSsl);
    void cancel();

signals:
    void loginUrlReady(const QString &url);
    void finished(const QString &server, const QString &loginName,
                  const QString &appPassword);
    void failed(const QString &error);

private:
    void onHttpFinished(int status, const QByteArray &body, const QString &etag,
                        const QUrl &url);
    void onHttpFailed(const QString &error);
    void handleStart(int status, const QByteArray &body);
    void handlePoll(int status, const QByteArray &body);
    void pollOnce();
    void onDeadline();

    HttpClient m_http;
    QTimer m_timer;
    // Bounds the polling, which the server answers the same whether the token
    // is still waiting for approval or already gone.
    QTimer m_deadline;
    QUrl m_pollUrl;
    QString m_token;
    bool m_starting;
    bool m_active;
};

#endif
