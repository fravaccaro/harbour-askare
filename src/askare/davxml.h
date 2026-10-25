#ifndef ASKARE_DAVXML_H
#define ASKARE_DAVXML_H

#include <QByteArray>
#include <QString>
#include <QVector>

#include "types.h"

namespace DavXml {

QString currentUserPrincipal(const QByteArray &xml);
QString calendarHomeSet(const QByteArray &xml);
QVector<CalendarInfo> calendars(const QByteArray &xml);
QVector<RemoteTask> report(const QByteArray &xml);

}

#endif
