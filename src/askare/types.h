#ifndef ASKARE_TYPES_H
#define ASKARE_TYPES_H

#include <QString>

struct CalendarInfo
{
    QString href;
    QString displayName;
    QString ctag;
    QString syncToken;
    bool allowsTodos;

    CalendarInfo()
        : allowsTodos(true)
    {
    }
};

struct RemoteTask
{
    QString href;
    QString etag;
    QString ics;
};

struct LocalTask
{
    QString href;
    QString calendarHref;
    QString uid;
    QString etag;
    QString ics;
    QString summary;
    QString description;
    QString status;
    QString completed;
    QString due;
    int priority;
    bool dirty;
    bool deleted;

    LocalTask()
        : priority(0)
        , dirty(false)
        , deleted(false)
    {
    }
};

struct TodoFields
{
    QString uid;
    QString summary;
    QString description;
    QString status;
    QString completed;
    QString due;
    int priority;

    TodoFields()
        : priority(0)
    {
    }
};

#endif
