#include "davclient.h"

#include <QDebug>

#include "davxml.h"
#include "url.h"

namespace {

const QByteArray PrincipalBody =
        "<d:propfind xmlns:d=\"DAV:\">"
        "<d:prop><d:current-user-principal/></d:prop>"
        "</d:propfind>";

const QByteArray HomeBody =
        "<d:propfind xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:prop><c:calendar-home-set/></d:prop>"
        "</d:propfind>";

const QByteArray ListBody =
        "<d:propfind xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\""
        " xmlns:cs=\"http://calendarserver.org/ns/\""
        " xmlns:a=\"http://apple.com/ns/ical/\">"
        "<d:prop>"
        "<d:displayname/>"
        "<d:resourcetype/>"
        "<c:supported-calendar-component-set/>"
        "<a:calendar-color/>"
        "<cs:getctag/>"
        "<d:sync-token/>"
        "</d:prop>"
        "</d:propfind>";

const QByteArray ReportBody =
        "<c:calendar-query xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:prop><d:getetag/><c:calendar-data/></d:prop>"
        "<c:filter><c:comp-filter name=\"VCALENDAR\">"
        "<c:comp-filter name=\"VTODO\"/>"
        "</c:comp-filter></c:filter>"
        "</c:calendar-query>";

HttpClient::Headers xmlHeaders(const QByteArray &depth)
{
    HttpClient::Headers headers;
    headers.append(qMakePair(QByteArray("Content-Type"),
                             QByteArray("application/xml; charset=utf-8")));
    headers.append(qMakePair(QByteArray("Depth"), depth));
    headers.append(qMakePair(QByteArray("Prefer"), QByteArray("return-minimal")));
    return headers;
}

}

DavClient::DavClient(QObject *parent)
    : QObject(parent)
    , m_operation(Idle)
{
    m_http.setUserAgent(QStringLiteral("Askare"));
    connect(&m_http, &HttpClient::finished, this, &DavClient::onFinished);
    connect(&m_http, &HttpClient::failed, this, &DavClient::onFailed);
}

void DavClient::setServer(const QString &serverUrl)
{
    m_server = QUrl(Url::normalizeServer(serverUrl));
}

void DavClient::setCredentials(const QString &user, const QString &password)
{
    m_http.setBasicAuth(user, password);
}

void DavClient::setIgnoreSslErrors(bool ignore)
{
    m_http.setIgnoreSslErrors(ignore);
}

QUrl DavClient::resolve(const QString &href) const
{
    return Url::join(m_server, href);
}

void DavClient::propfind(const QUrl &url, const QByteArray &body)
{
    m_http.send("PROPFIND", url, body, xmlHeaders("0"));
}

void DavClient::discoverPrincipal()
{
    m_operation = Principal;
    propfind(QUrl(Url::origin(m_server) + QStringLiteral("/remote.php/dav/")),
             PrincipalBody);
}

void DavClient::discoverHome(const QString &principalHref)
{
    m_operation = Home;
    propfind(resolve(principalHref), HomeBody);
}

void DavClient::listCalendars(const QString &homeHref)
{
    m_operation = List;
    m_http.send("PROPFIND", resolve(homeHref), ListBody, xmlHeaders("1"));
}

void DavClient::fetchTodos(const QString &calendarHref)
{
    m_operation = Report;
    m_http.send("REPORT", resolve(calendarHref), ReportBody, xmlHeaders("1"));
}

void DavClient::upload(const QString &href, const QString &ics, const QString &etag)
{
    m_operation = Put;
    m_pendingHref = href;
    HttpClient::Headers headers;
    headers.append(qMakePair(QByteArray("Content-Type"),
                             QByteArray("text/calendar; charset=utf-8")));
    if (etag.isEmpty())
        headers.append(qMakePair(QByteArray("If-None-Match"), QByteArray("*")));
    else
        headers.append(qMakePair(QByteArray("If-Match"), etag.toUtf8()));
    m_http.send("PUT", resolve(href), ics.toUtf8(), headers);
}

void DavClient::remove(const QString &href, const QString &etag)
{
    m_operation = Del;
    m_pendingHref = href;
    HttpClient::Headers headers;
    if (!etag.isEmpty())
        headers.append(qMakePair(QByteArray("If-Match"), etag.toUtf8()));
    m_http.send("DELETE", resolve(href), QByteArray(), headers);
}

void DavClient::failStatus(int status)
{
    m_operation = Idle;
    emit failed(tr("The server answered with %1.").arg(status));
}

void DavClient::onFailed(const QString &error)
{
    m_operation = Idle;
    emit failed(error);
}

void DavClient::onFinished(int status, const QByteArray &body, const QString &etag,
                           const QUrl &)
{
    switch (m_operation) {
    case Principal:
        handlePrincipal(status, body);
        return;
    case Home:
        handleHome(status, body);
        return;
    case List:
        handleList(status, body);
        return;
    case Report:
        handleReport(status, body);
        return;
    case Put:
        handlePut(status, body, etag);
        return;
    case Del:
        handleDelete(status);
        return;
    case Idle:
        break;
    }
}

void DavClient::handlePrincipal(int status, const QByteArray &body)
{
    if (status != 207) {
        failStatus(status);
        return;
    }
    const QString href = DavXml::currentUserPrincipal(body);
    m_operation = Idle;
    if (href.isEmpty()) {
        emit failed(tr("The calendar account could not be found."));
        return;
    }
    emit principalReady(href);
}

void DavClient::handleHome(int status, const QByteArray &body)
{
    if (status != 207) {
        failStatus(status);
        return;
    }
    const QString href = DavXml::calendarHomeSet(body);
    m_operation = Idle;
    if (href.isEmpty()) {
        emit failed(tr("The task lists could not be found."));
        return;
    }
    emit homeReady(href);
}

void DavClient::handleList(int status, const QByteArray &body)
{
    if (status != 207) {
        failStatus(status);
        return;
    }
    m_operation = Idle;
    emit calendarsReady(DavXml::calendars(body));
}

void DavClient::handleReport(int status, const QByteArray &body)
{
    if (status != 207) {
        failStatus(status);
        return;
    }
    m_operation = Idle;
    emit todosReady(DavXml::report(body));
}

void DavClient::handlePut(int status, const QByteArray &body, const QString &etag)
{
    const QString href = m_pendingHref;
    m_operation = Idle;
    if (status != 200 && status != 201 && status != 204) {
        const QString detail = DavXml::errorMessage(body);
        qWarning() << "askare: PUT" << resolve(href) << "status" << status << detail;
        emit failed(detail.isEmpty()
                    ? tr("The task could not be saved (%1).").arg(status)
                    : detail);
        return;
    }
    emit uploaded(href, etag);
}

void DavClient::handleDelete(int status)
{
    const QString href = m_pendingHref;
    m_operation = Idle;
    if (status != 200 && status != 204 && status != 404) {
        emit failed(tr("The task could not be deleted (%1).").arg(status));
        return;
    }
    emit removed(href);
}
