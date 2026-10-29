#ifndef ASKARE_ICS_H
#define ASKARE_ICS_H

#include <QDateTime>
#include <QString>

#include "types.h"

namespace Ics {

QString unfold(const QString &text);
QString fold(const QString &text);
QString unescape(const QString &value);
QString escape(const QString &value);
QString propertyName(const QString &line);
QString propertyValue(const QString &line);
TodoFields parse(const QString &ics);
QString rewrite(const QString &ics, const TodoFields &fields);
QString create(const TodoFields &fields);
QString normalize(const QString &ics);
QString nowStamp();
QString newUid();
QString dueFromDate(const QString &isoDate);
QString dueFromDateTime(const QString &isoDate, const QString &time);
QString dateFromDue(const QString &due);
QString timeFromDue(const QString &due);
QDateTime dueDateTime(const QString &due);
QDateTime reminderAt(const QString &due, int minutesBefore);
bool isCompleted(const QString &status);
bool isCancelled(const QString &status);
bool isCurrent(const QString &status, const QString &due);
bool isImportant(const QString &status, int priority);
QString statusFromPercent(int percent);
int percentFromStatus(const QString &status, int percent);

}

#endif
