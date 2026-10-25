#include "davxml.h"

#include <QUrl>
#include <QXmlStreamReader>

namespace {

bool localIs(const QXmlStreamReader &reader, const char *name)
{
    return reader.name() == QLatin1String(name);
}

QString readText(QXmlStreamReader *reader)
{
    return reader->readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
}

QString readHref(QXmlStreamReader *reader)
{
    while (!reader->atEnd()) {
        reader->readNext();
        if (localIs(*reader, "href") && reader->isStartElement())
            return QUrl::fromPercentEncoding(readText(reader).toUtf8());
        if (reader->isEndElement() && (localIs(*reader, "current-user-principal")
                                       || localIs(*reader, "calendar-home-set")))
            break;
    }
    return QString();
}

void readResponse(QXmlStreamReader *reader, QVector<CalendarInfo> *out)
{
    CalendarInfo info;
    bool isCalendar = false;
    while (!reader->atEnd()) {
        reader->readNext();
        if (reader->isEndElement() && localIs(*reader, "response"))
            break;
        if (!reader->isStartElement())
            continue;
        if (localIs(*reader, "href") && info.href.isEmpty())
            info.href = QUrl::fromPercentEncoding(readText(reader).toUtf8());
        else if (localIs(*reader, "displayname"))
            info.displayName = readText(reader);
        else if (localIs(*reader, "getctag"))
            info.ctag = readText(reader);
        else if (localIs(*reader, "sync-token"))
            info.syncToken = readText(reader);
        else if (localIs(*reader, "calendar"))
            isCalendar = true;
    }
    if (isCalendar)
        out->append(info);
}

void readReportResponse(QXmlStreamReader *reader, QVector<RemoteTask> *out)
{
    RemoteTask task;
    while (!reader->atEnd()) {
        reader->readNext();
        if (localIs(*reader, "href") && reader->isStartElement())
            task.href = QUrl::fromPercentEncoding(readText(reader).toUtf8());
        else if (localIs(*reader, "getetag") && reader->isStartElement())
            task.etag = readText(reader);
        else if (localIs(*reader, "calendar-data") && reader->isStartElement())
            task.ics = readText(reader);
        else if (localIs(*reader, "response") && reader->isEndElement())
            break;
    }
    if (!task.href.isEmpty() && !task.ics.isEmpty())
        out->append(task);
}

QString firstNamedHref(const QByteArray &xml, const char *parent)
{
    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();
        if (localIs(reader, parent) && reader.isStartElement())
            return readHref(&reader);
    }
    return QString();
}

}

namespace DavXml {

QString currentUserPrincipal(const QByteArray &xml)
{
    return firstNamedHref(xml, "current-user-principal");
}

QString calendarHomeSet(const QByteArray &xml)
{
    return firstNamedHref(xml, "calendar-home-set");
}

QVector<CalendarInfo> calendars(const QByteArray &xml)
{
    QVector<CalendarInfo> out;
    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();
        if (localIs(reader, "response") && reader.isStartElement())
            readResponse(&reader, &out);
    }
    return out;
}

QVector<RemoteTask> report(const QByteArray &xml)
{
    QVector<RemoteTask> out;
    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();
        if (localIs(reader, "response") && reader.isStartElement())
            readReportResponse(&reader, &out);
    }
    return out;
}

QString errorMessage(const QByteArray &xml)
{
    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();
        if (localIs(reader, "message") && reader.isStartElement())
            return readText(&reader);
    }
    return QString();
}

}
