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

void readComponentSet(QXmlStreamReader *reader, CalendarInfo *info, bool *sawSet)
{
    *sawSet = true;
    info->allowsTodos = false;
    while (!reader->atEnd()) {
        reader->readNext();
        if (localIs(*reader, "comp") && reader->isStartElement()) {
            if (reader->attributes().value(QStringLiteral("name")) == QLatin1String("VTODO"))
                info->allowsTodos = true;
        } else if (localIs(*reader, "supported-calendar-component-set")
                   && reader->isEndElement()) {
            break;
        }
    }
}

void readResourceType(QXmlStreamReader *reader, bool *isCalendar)
{
    *isCalendar = false;
    while (!reader->atEnd()) {
        reader->readNext();
        if (localIs(*reader, "calendar") && reader->isStartElement())
            *isCalendar = true;
        else if (localIs(*reader, "resourcetype") && reader->isEndElement())
            break;
    }
}

void readProp(QXmlStreamReader *reader, CalendarInfo *info, bool *isCalendar)
{
    bool sawSet = false;
    info->allowsTodos = true;
    while (!reader->atEnd()) {
        reader->readNext();
        if (localIs(*reader, "displayname") && reader->isStartElement())
            info->displayName = readText(reader);
        else if (localIs(*reader, "getctag") && reader->isStartElement())
            info->ctag = readText(reader);
        else if (localIs(*reader, "sync-token") && reader->isStartElement())
            info->syncToken = readText(reader);
        else if (localIs(*reader, "resourcetype") && reader->isStartElement())
            readResourceType(reader, isCalendar);
        else if (localIs(*reader, "supported-calendar-component-set")
                 && reader->isStartElement())
            readComponentSet(reader, info, &sawSet);
        else if (localIs(*reader, "prop") && reader->isEndElement())
            break;
    }
}

void readResponse(QXmlStreamReader *reader, QVector<CalendarInfo> *out)
{
    CalendarInfo info;
    bool isCalendar = false;
    while (!reader->atEnd()) {
        reader->readNext();
        if (localIs(*reader, "href") && reader->isStartElement())
            info.href = QUrl::fromPercentEncoding(readText(reader).toUtf8());
        else if (localIs(*reader, "prop") && reader->isStartElement())
            readProp(reader, &info, &isCalendar);
        else if (localIs(*reader, "response") && reader->isEndElement())
            break;
    }
    if (isCalendar && info.allowsTodos)
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

}
