#include "ics.h"

#include <QDate>
#include <QDateTime>
#include <QStringList>
#include <QUuid>

namespace {

const char *kKnown[] = {
    "UID", "SUMMARY", "DESCRIPTION", "STATUS", "COMPLETED", "DUE", "PRIORITY",
    "DTSTAMP"
};

bool isKnown(const QString &name)
{
    for (int i = 0; i < 8; ++i) {
        if (name == QLatin1String(kKnown[i]))
            return true;
    }
    return false;
}

QString foldedLine(const QString &line)
{
    if (line.size() <= 75)
        return line;

    QString out = line.left(75);
    int index = 75;
    while (index < line.size()) {
        out += QStringLiteral("\r\n ");
        out += line.mid(index, 74);
        index += 74;
    }
    return out;
}

QString fieldLine(const QString &name, const QString &value)
{
    if (value.isEmpty())
        return QString();

    return name + QLatin1Char(':') + value;
}

QStringList todoBlock(const QStringList &lines)
{
    QStringList block;
    bool inside = false;
    for (const QString &line : lines) {
        if (line.startsWith(QLatin1String("BEGIN:VTODO")))
            inside = true;
        if (inside)
            block.append(line);
        if (line.startsWith(QLatin1String("END:VTODO")))
            break;
    }
    return block;
}

QString valueOrEmpty(const TodoFields &fields, const QString &name)
{
    if (name == QLatin1String("UID"))
        return fields.uid;
    if (name == QLatin1String("SUMMARY"))
        return Ics::escape(fields.summary);
    if (name == QLatin1String("DESCRIPTION"))
        return Ics::escape(fields.description);
    if (name == QLatin1String("STATUS"))
        return fields.status;
    if (name == QLatin1String("COMPLETED"))
        return fields.completed;
    if (name == QLatin1String("DUE"))
        return fields.due;
    if (name == QLatin1String("PRIORITY"))
        return fields.priority > 0 ? QString::number(fields.priority) : QString();
    if (name == QLatin1String("DTSTAMP"))
        return Ics::nowStamp();
    return QString();
}

void applyLine(TodoFields *fields, const QString &name, const QString &value)
{
    if (name == QLatin1String("UID"))
        fields->uid = value;
    else if (name == QLatin1String("SUMMARY"))
        fields->summary = Ics::unescape(value);
    else if (name == QLatin1String("DESCRIPTION"))
        fields->description = Ics::unescape(value);
    else if (name == QLatin1String("STATUS"))
        fields->status = value;
    else if (name == QLatin1String("COMPLETED"))
        fields->completed = value;
    else if (name == QLatin1String("DUE"))
        fields->due = value;
    else if (name == QLatin1String("PRIORITY"))
        fields->priority = value.toInt();
}

}

namespace Ics {

QString unfold(const QString &text)
{
    QString normalized = text;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    normalized.replace(QStringLiteral("\n "), QString());
    normalized.replace(QStringLiteral("\n\t"), QString());
    return normalized;
}

QString fold(const QString &text)
{
    const QStringList lines = text.split(QLatin1Char('\n'), QString::SkipEmptyParts);
    QStringList folded;
    for (const QString &line : lines)
        folded.append(foldedLine(line));
    return folded.join(QStringLiteral("\r\n")) + QStringLiteral("\r\n");
}

QString unescape(const QString &value)
{
    QString out = value;
    out.replace(QStringLiteral("\\n"), QStringLiteral("\n"));
    out.replace(QStringLiteral("\\N"), QStringLiteral("\n"));
    out.replace(QStringLiteral("\\,"), QStringLiteral(","));
    out.replace(QStringLiteral("\\;"), QStringLiteral(";"));
    out.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
    return out;
}

QString escape(const QString &value)
{
    QString out = value;
    out.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    out.replace(QLatin1Char(';'), QStringLiteral("\\;"));
    out.replace(QLatin1Char(','), QStringLiteral("\\,"));
    out.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
    return out;
}

QString propertyName(const QString &line)
{
    const int colon = line.indexOf(QLatin1Char(':'));
    const QString head = colon >= 0 ? line.left(colon) : line;
    const int semi = head.indexOf(QLatin1Char(';'));
    return (semi >= 0 ? head.left(semi) : head).toUpper();
}

QString propertyValue(const QString &line)
{
    const int colon = line.indexOf(QLatin1Char(':'));
    if (colon < 0)
        return QString();

    return line.mid(colon + 1);
}

TodoFields parse(const QString &ics)
{
    TodoFields fields;
    fields.priority = 0;
    const QStringList lines = unfold(ics).split(QLatin1Char('\n'), QString::SkipEmptyParts);
    for (const QString &line : todoBlock(lines))
        applyLine(&fields, propertyName(line), propertyValue(line));
    return fields;
}

void appendMissing(QStringList *out, QStringList *seen, const TodoFields &fields)
{
    for (int i = 0; i < 8; ++i) {
        const QString name = QLatin1String(kKnown[i]);
        if (seen->contains(name))
            continue;
        const QString next = fieldLine(name, valueOrEmpty(fields, name));
        if (!next.isEmpty())
            out->append(next);
    }
}

QString rewrite(const QString &ics, const TodoFields &fields)
{
    const QStringList lines = unfold(ics).split(QLatin1Char('\n'), QString::SkipEmptyParts);
    QStringList out;
    QStringList seen;
    bool inTodo = false;
    for (const QString &line : lines) {
        if (line.startsWith(QLatin1String("BEGIN:VTODO"))) {
            inTodo = true;
            out.append(line);
            continue;
        }
        if (line.startsWith(QLatin1String("END:VTODO"))) {
            appendMissing(&out, &seen, fields);
            inTodo = false;
            out.append(line);
            continue;
        }
        const QString name = propertyName(line);
        if (inTodo && isKnown(name)) {
            if (seen.contains(name))
                continue;
            seen.append(name);
            const QString next = fieldLine(name, valueOrEmpty(fields, name));
            if (!next.isEmpty())
                out.append(next);
            continue;
        }
        out.append(line);
    }
    return fold(out.join(QLatin1Char('\n')));
}

QString create(const TodoFields &fields)
{
    TodoFields copy = fields;
    if (copy.uid.isEmpty())
        copy.uid = newUid();
    if (copy.status.isEmpty())
        copy.status = QStringLiteral("NEEDS-ACTION");

    QStringList lines;
    lines << QStringLiteral("BEGIN:VCALENDAR")
          << QStringLiteral("VERSION:2.0")
          << QStringLiteral("PRODID:-//Askare//EN")
          << QStringLiteral("BEGIN:VTODO")
          << fieldLine(QStringLiteral("UID"), copy.uid)
          << fieldLine(QStringLiteral("DTSTAMP"), nowStamp())
          << fieldLine(QStringLiteral("SUMMARY"), escape(copy.summary))
          << fieldLine(QStringLiteral("DESCRIPTION"), escape(copy.description))
          << fieldLine(QStringLiteral("STATUS"), copy.status)
          << fieldLine(QStringLiteral("COMPLETED"), copy.completed)
          << fieldLine(QStringLiteral("DUE"), copy.due);
    if (copy.priority > 0)
        lines << fieldLine(QStringLiteral("PRIORITY"), QString::number(copy.priority));
    lines << QStringLiteral("END:VTODO")
          << QStringLiteral("END:VCALENDAR");
    return fold(lines.join(QLatin1Char('\n')));
}

QString nowStamp()
{
    return QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMddTHHmmssZ"));
}

QString newUid()
{
    return QUuid::createUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

QString dueFromDate(const QString &isoDate)
{
    return QString(isoDate).remove(QLatin1Char('-'));
}

QString dateFromDue(const QString &due)
{
    if (due.size() < 8)
        return QString();

    return due.left(4) + QLatin1Char('-') + due.mid(4, 2) + QLatin1Char('-') + due.mid(6, 2);
}

bool isCompleted(const QString &status)
{
    return status.compare(QLatin1String("COMPLETED"), Qt::CaseInsensitive) == 0;
}

bool isCurrent(const QString &status, const QString &due)
{
    if (isCompleted(status))
        return false;
    if (due.isEmpty())
        return true;

    const QString today = QDate::currentDate().toString(QStringLiteral("yyyyMMdd"));
    return due.left(8) <= today;
}

}
