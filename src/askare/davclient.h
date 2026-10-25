#ifndef ASKARE_DAVCLIENT_H
#define ASKARE_DAVCLIENT_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVector>

#include "httpclient.h"
#include "types.h"

class DavClient : public QObject
{
    Q_OBJECT

public:
    enum Operation {
        Idle,
        Principal,
        Home,
        List,
        Report,
        Put,
        Del
    };

    explicit DavClient(QObject *parent = nullptr);

    void setServer(const QString &serverUrl);
    void setCredentials(const QString &user, const QString &password);
    void setIgnoreSslErrors(bool ignore);

    void discoverPrincipal();
    void discoverHome(const QString &principalHref);
    void listCalendars(const QString &homeHref);
    void fetchTodos(const QString &calendarHref);
    void upload(const QString &href, const QString &ics, const QString &etag);
    void remove(const QString &href, const QString &etag);

signals:
    void principalReady(const QString &href);
    void homeReady(const QString &href);
    void calendarsReady(const QVector<CalendarInfo> &calendars);
    void todosReady(const QVector<RemoteTask> &tasks);
    void uploaded(const QString &href, const QString &etag);
    void removed(const QString &href);
    void failed(const QString &error);

private:
    void onFinished(int status, const QByteArray &body, const QString &etag,
                    const QUrl &url);
    void onFailed(const QString &error);
    QUrl resolve(const QString &href) const;
    void propfind(const QUrl &url, const QByteArray &body);
    void handlePrincipal(int status, const QByteArray &body);
    void handleHome(int status, const QByteArray &body);
    void handleList(int status, const QByteArray &body);
    void handleReport(int status, const QByteArray &body);
    void handlePut(int status, const QString &etag);
    void handleDelete(int status);
    void failStatus(int status);

    HttpClient m_http;
    QUrl m_server;
    QString m_pendingHref;
    Operation m_operation;
};

#endif
