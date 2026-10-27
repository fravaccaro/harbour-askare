#include "ics.h"

#include <QDate>
#include <QDateTime>
#include <QRegularExpression>
#include <QStringList>
#include <QTime>
#include <QUuid>

namespace {

const char *kKnown[] = {
    "UID", "SUMMARY", "DESCRIPTION", "STATUS", "COMPLETED", "DTSTART", "DUE",
    "PRIORITY", "PERCENT-COMPLETE", "DTSTAMP"
};

const int kKnownCount = sizeof(kKnown) / sizeof(kKnown[0]);

// A point in time the user picks by day, so it is written as a date unless a
// time of day was set.
bool isDateOrTime(const QString &name)
{
    return name == QLatin1String("DUE") || name == QLatin1String("DTSTART");
}

bool isKnown(const QString &name)
{
    for (int i = 0; i < kKnownCount; ++i) {
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
    if (isDateOrTime(name) && !value.contains(QLatin1Char('T')))
        return name + QStringLiteral(";VALUE=DATE:") + value.left(8);
    if (name == QLatin1String("SUMMARY") || name == QLatin1String("DESCRIPTION"))
        return name + QLatin1Char(':') + Ics::escape(value);
    return name + QLatin1Char(':') + value;
}

void appendLine(QStringList *lines, const QString &line)
{
    if (!line.isEmpty())
        lines->append(line);
}

// Minutes before the due time, or -1 when the trigger is absolute, points
// after the due time, or cannot be read. Such alarms are left untouched.
int durationToMinutes(const QString &value)
{
    const QRegularExpression pattern(
        QStringLiteral("^(-)?P(?:(\\d+)W)?(?:(\\d+)D)?"
                       "(?:T(?:(\\d+)H)?(?:(\\d+)M)?(?:(\\d+)S)?)?$"));
    const QRegularExpressionMatch match = pattern.match(value.trimmed());
    if (!match.hasMatch())
        return -1;

    const int minutes = match.captured(2).toInt() * 7 * 24 * 60
            + match.captured(3).toInt() * 24 * 60
            + match.captured(4).toInt() * 60
            + match.captured(5).toInt()
            + match.captured(6).toInt() / 60;
    if (minutes == 0)
        return 0;
    return match.captured(1).isEmpty() ? -1 : minutes;
}

QString minutesToDuration(int minutes)
{
    if (minutes <= 0)
        return QStringLiteral("PT0S");
    if (minutes % (24 * 60) == 0)
        return QStringLiteral("-P%1D").arg(minutes / (24 * 60));
    if (minutes % 60 == 0)
        return QStringLiteral("-PT%1H").arg(minutes / 60);
    return QStringLiteral("-PT%1M").arg(minutes);
}

// RELATED-TO may appear several times with different link types, so only the
// parent entry belongs to Askare. A missing RELTYPE means PARENT.
bool isParentLink(const QString &line)
{
    if (Ics::propertyName(line) != QLatin1String("RELATED-TO"))
        return false;

    const int colon = line.indexOf(QLatin1Char(':'));
    const QString params = colon < 0 ? line : line.left(colon);
    return !params.contains(QLatin1String("RELTYPE="))
            || params.contains(QLatin1String("RELTYPE=PARENT"));
}

QString parentLine(const TodoFields &fields)
{
    if (fields.parentUid.isEmpty())
        return QString();

    return QStringLiteral("RELATED-TO;RELTYPE=PARENT:") + fields.parentUid;
}

bool isSubComponentStart(const QString &line)
{
    return line.startsWith(QLatin1String("BEGIN:"));
}

bool isSubComponentEnd(const QString &line)
{
    return line.startsWith(QLatin1String("END:"));
}

// An alarm Askare can represent: a display alarm with a relative trigger.
bool isManagedAlarm(const QStringList &block)
{
    if (block.isEmpty() || !block.first().startsWith(QLatin1String("BEGIN:VALARM")))
        return false;

    for (const QString &line : block) {
        if (Ics::propertyName(line) == QLatin1String("TRIGGER"))
            return durationToMinutes(Ics::propertyValue(line)) >= 0;
    }
    return false;
}

QStringList alarmBlock(const TodoFields &fields)
{
    QStringList block;
    if (fields.reminder < 0)
        return block;

    const QString text = fields.summary.isEmpty() ? QStringLiteral("Askare")
                                                  : Ics::escape(fields.summary);
    block << QStringLiteral("BEGIN:VALARM")
          << QStringLiteral("ACTION:DISPLAY")
          << QStringLiteral("DESCRIPTION:") + text
          << QStringLiteral("TRIGGER;RELATED=END:") + minutesToDuration(fields.reminder)
          << QStringLiteral("END:VALARM");
    return block;
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
    if (name == QLatin1String("DTSTART"))
        return fields.start;
    if (name == QLatin1String("DUE"))
        return fields.due;
    if (name == QLatin1String("PRIORITY"))
        return fields.priority > 0 ? QString::number(fields.priority) : QString();
    if (name == QLatin1String("PERCENT-COMPLETE"))
        return fields.percent > 0 ? QString::number(fields.percent) : QString();
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
    else if (name == QLatin1String("DTSTART"))
        fields->start = value;
    else if (name == QLatin1String("DUE"))
        fields->due = value;
    else if (name == QLatin1String("PRIORITY"))
        fields->priority = value.toInt();
    else if (name == QLatin1String("PERCENT-COMPLETE"))
        fields->percent = value.toInt();
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
    fields.percent = 0;
    fields.reminder = -1;
    const QStringList lines = unfold(ics).split(QLatin1Char('\n'), QString::SkipEmptyParts);
    int depth = 0;
    for (const QString &line : todoBlock(lines)) {
        if (isSubComponentStart(line)) {
            ++depth;
            continue;
        }
        if (isSubComponentEnd(line)) {
            depth = depth > 0 ? depth - 1 : 0;
            continue;
        }
        // Properties of a nested VALARM must not be read as the task's own.
        if (depth > 1) {
            if (fields.reminder < 0 && propertyName(line) == QLatin1String("TRIGGER"))
                fields.reminder = durationToMinutes(propertyValue(line));
            continue;
        }
        // The link type lives in the parameters, so this needs the whole line.
        if (isParentLink(line)) {
            if (fields.parentUid.isEmpty())
                fields.parentUid = propertyValue(line);
            continue;
        }
        applyLine(&fields, propertyName(line), propertyValue(line));
    }
    return fields;
}

void appendMissing(QStringList *out, QStringList *seen, const TodoFields &fields)
{
    for (int i = 0; i < kKnownCount; ++i) {
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
    QStringList nested;
    QStringList kept;
    bool inTodo = false;
    bool replacedAlarm = false;
    int depth = 0;
    for (const QString &line : lines) {
        if (inTodo && depth == 0 && isSubComponentStart(line)) {
            depth = 1;
            nested.clear();
            nested.append(line);
            continue;
        }
        if (depth > 0) {
            nested.append(line);
            if (isSubComponentStart(line))
                ++depth;
            else if (isSubComponentEnd(line))
                --depth;
            if (depth == 0) {
                // The alarm Askare manages is rebuilt below; keep any other.
                if (!replacedAlarm && isManagedAlarm(nested))
                    replacedAlarm = true;
                else
                    kept.append(nested);
                nested.clear();
            }
            continue;
        }
        if (line.startsWith(QLatin1String("BEGIN:VTODO"))) {
            inTodo = true;
            out.append(line);
            continue;
        }
        if (line.startsWith(QLatin1String("END:VTODO"))) {
            // A VTODO body is properties first, then its alarms.
            appendMissing(&out, &seen, fields);
            appendLine(&out, parentLine(fields));
            out.append(kept);
            out.append(alarmBlock(fields));
            kept.clear();
            inTodo = false;
            out.append(line);
            continue;
        }
        // Only the parent link is rebuilt; other link types pass through.
        if (inTodo && isParentLink(line))
            continue;
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

QString normalize(const QString &ics)
{
    const QStringList lines = unfold(ics).split(QLatin1Char('\n'), QString::SkipEmptyParts);
    QStringList out;
    for (const QString &line : lines) {
        const QString name = propertyName(line);
        if (isDateOrTime(name))
            out.append(fieldLine(name, propertyValue(line)));
        else
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
    const QString stamp = nowStamp();
    lines << QStringLiteral("BEGIN:VCALENDAR")
          << QStringLiteral("VERSION:2.0")
          << QStringLiteral("PRODID:-//Askare//Askare//EN")
          << QStringLiteral("CALSCALE:GREGORIAN")
          << QStringLiteral("BEGIN:VTODO");
    appendLine(&lines, fieldLine(QStringLiteral("UID"), copy.uid));
    appendLine(&lines, fieldLine(QStringLiteral("DTSTAMP"), stamp));
    appendLine(&lines, fieldLine(QStringLiteral("CREATED"), stamp));
    appendLine(&lines, fieldLine(QStringLiteral("LAST-MODIFIED"), stamp));
    appendLine(&lines, fieldLine(QStringLiteral("SUMMARY"), copy.summary));
    appendLine(&lines, fieldLine(QStringLiteral("DESCRIPTION"), copy.description));
    appendLine(&lines, fieldLine(QStringLiteral("STATUS"), copy.status));
    appendLine(&lines, fieldLine(QStringLiteral("COMPLETED"), copy.completed));
    appendLine(&lines, fieldLine(QStringLiteral("DTSTART"), copy.start));
    appendLine(&lines, fieldLine(QStringLiteral("DUE"), copy.due));
    appendLine(&lines, fieldLine(QStringLiteral("PRIORITY"),
                                 copy.priority > 0 ? QString::number(copy.priority)
                                                   : QString()));
    appendLine(&lines, fieldLine(QStringLiteral("PERCENT-COMPLETE"),
                                 copy.percent > 0 ? QString::number(copy.percent)
                                                  : QString()));
    appendLine(&lines, parentLine(copy));
    lines << alarmBlock(copy)
          << QStringLiteral("END:VTODO")
          << QStringLiteral("END:VCALENDAR");
    return fold(lines.join(QLatin1Char('\n')));
}

QString nowStamp()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    return now.toString(QStringLiteral("yyyyMMdd")) + QLatin1Char('T')
            + now.toString(QStringLiteral("HHmmss")) + QLatin1Char('Z');
}

QString newUid()
{
    return QUuid::createUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

QString dueFromDate(const QString &isoDate)
{
    return QString(isoDate).remove(QLatin1Char('-'));
}

QString dueFromDateTime(const QString &isoDate, const QString &time)
{
    const QString date = dueFromDate(isoDate);
    if (date.isEmpty() || time.isEmpty())
        return date;

    return date + QLatin1Char('T') + QString(time).remove(QLatin1Char(':'))
            + QStringLiteral("00");
}

QString dateFromDue(const QString &due)
{
    if (due.size() < 8)
        return QString();

    return due.left(4) + QLatin1Char('-') + due.mid(4, 2) + QLatin1Char('-') + due.mid(6, 2);
}

QString timeFromDue(const QString &due)
{
    const int mark = due.indexOf(QLatin1Char('T'));
    if (mark < 0 || due.size() < mark + 5)
        return QString();

    return due.mid(mark + 1, 2) + QLatin1Char(':') + due.mid(mark + 3, 2);
}

QDateTime dueDateTime(const QString &due)
{
    const QDate date = QDate::fromString(due.left(8), QStringLiteral("yyyyMMdd"));
    if (!date.isValid())
        return QDateTime();

    const int mark = due.indexOf(QLatin1Char('T'));
    if (mark < 0)
        return QDateTime(date, QTime(0, 0));

    const QTime time = QTime::fromString(due.mid(mark + 1, 6), QStringLiteral("HHmmss"));
    QDateTime stamp(date, time.isValid() ? time : QTime(0, 0));
    if (due.endsWith(QLatin1Char('Z'))) {
        stamp.setTimeSpec(Qt::UTC);
        return stamp.toLocalTime();
    }
    return stamp;
}

QDateTime reminderAt(const QString &due, int minutesBefore)
{
    if (minutesBefore < 0)
        return QDateTime();

    const QDateTime stamp = dueDateTime(due);
    return stamp.isValid() ? stamp.addSecs(-minutesBefore * 60) : QDateTime();
}

bool isCompleted(const QString &status)
{
    return status.compare(QLatin1String("COMPLETED"), Qt::CaseInsensitive) == 0;
}

QString statusFromPercent(int percent)
{
    if (percent >= 100)
        return QStringLiteral("COMPLETED");
    if (percent > 0)
        return QStringLiteral("IN-PROCESS");
    return QStringLiteral("NEEDS-ACTION");
}

int percentFromStatus(const QString &status, int percent)
{
    if (isCompleted(status))
        return 100;
    return percent;
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

// Nextcloud's Important list: an incomplete task in the high band (1-4).
// Priority 5 is medium and 6-9 low, so neither counts.
bool isImportant(const QString &status, int priority)
{
    return !isCompleted(status) && priority >= 1 && priority <= 4;
}

}
