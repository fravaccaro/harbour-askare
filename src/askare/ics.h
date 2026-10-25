#ifndef ASKARE_ICS_H
#define ASKARE_ICS_H

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
QString dateFromDue(const QString &due);
bool isCompleted(const QString &status);
bool isCurrent(const QString &status, const QString &due);
QString statusFromPercent(int percent);
int percentFromStatus(const QString &status, int percent);

}

#endif
