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

QString normalizedColor(const QString &value)
{
    QString color = value.trimmed();
    if (!color.startsWith(QLatin1Char('#')))
        return QString();
    if (color.size() >= 9)
        color = color.left(7);
    return color.size() == 7 ? color : QString();
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

struct ResponseState
{
    CalendarInfo info;
    bool isCalendar;
    bool sawSet;
    bool sawTodo;
    bool sawPrivileges;
    bool canWrite;
    bool inPrivileges;

    ResponseState()
        : isCalendar(false)
        , sawSet(false)
        , sawTodo(false)
        , sawPrivileges(false)
        , canWrite(false)
        , inPrivileges(false)
    {
    }
};

bool isWritePrivilege(const QXmlStreamReader &reader)
{
    return localIs(reader, "write") || localIs(reader, "write-content")
            || localIs(reader, "bind") || localIs(reader, "all");
}

// A PROPFIND answer repeats missing properties as empty elements in a 404
// propstat block, so an empty value must never replace one already read.
void keepIfSet(QString *target, const QString &value)
{
    if (!value.isEmpty())
        *target = value;
}

void readProperty(ResponseState *state, QXmlStreamReader *reader)
{
    CalendarInfo &info = state->info;
    if (localIs(*reader, "href") && info.href.isEmpty())
        info.href = QUrl::fromPercentEncoding(readText(reader).toUtf8());
    else if (localIs(*reader, "displayname"))
        keepIfSet(&info.displayName, readText(reader));
    else if (localIs(*reader, "calendar-color"))
        keepIfSet(&info.color, normalizedColor(readText(reader)));
    else if (localIs(*reader, "getctag"))
        keepIfSet(&info.ctag, readText(reader));
    else if (localIs(*reader, "sync-token"))
        keepIfSet(&info.syncToken, readText(reader));
    else if (localIs(*reader, "calendar"))
        state->isCalendar = true;
    else if (localIs(*reader, "comp")) {
        state->sawSet = true;
        state->sawTodo = state->sawTodo
                || reader->attributes().value(QStringLiteral("name"))
                   .compare(QLatin1String("VTODO"), Qt::CaseInsensitive) == 0;
    }
}

void readResponse(QXmlStreamReader *reader, QVector<CalendarInfo> *out)
{
    ResponseState state;
    while (!reader->atEnd()) {
        reader->readNext();
        if (reader->isEndElement() && localIs(*reader, "response"))
            break;
        if (reader->isEndElement() && localIs(*reader, "current-user-privilege-set"))
            state.inPrivileges = false;
        if (!reader->isStartElement())
            continue;
        if (localIs(*reader, "current-user-privilege-set")) {
            state.inPrivileges = true;
        } else if (state.inPrivileges) {
            // Only a listed privilege counts; an empty set says nothing.
            state.sawPrivileges = true;
            state.canWrite = state.canWrite || isWritePrivilege(*reader);
        } else {
            readProperty(&state, reader);
        }
    }
    // Silence about components or privileges is taken as permission.
    state.info.allowsTodos = state.sawTodo || !state.sawSet;
    state.info.writable = state.canWrite || !state.sawPrivileges;
    if (state.isCalendar)
        out->append(state.info);
}

QVector<CalendarInfo> todoCalendars(const QVector<CalendarInfo> &all, bool requireWritable)
{
    QVector<CalendarInfo> out;
    for (const CalendarInfo &info : all) {
        if (info.allowsTodos && (!requireWritable || info.writable))
            out.append(info);
    }
    return out;
}

QVector<CalendarInfo> usableCalendars(const QVector<CalendarInfo> &all)
{
    const QVector<CalendarInfo> writable = todoCalendars(all, true);
    if (!writable.isEmpty())
        return writable;

    const QVector<CalendarInfo> anyTodo = todoCalendars(all, false);
    return anyTodo.isEmpty() ? all : anyTodo;
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
    return usableCalendars(out);
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
