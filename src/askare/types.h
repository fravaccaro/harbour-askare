#ifndef ASKARE_TYPES_H
#define ASKARE_TYPES_H

#include <QString>
#include <QStringList>

struct CalendarInfo
{
    QString href;
    QString displayName;
    QString color;
    QString ctag;
    QString syncToken;
    bool allowsTodos;
    bool writable;

    CalendarInfo()
        : allowsTodos(true)
        , writable(true)
    {
    }
};

// One tag and how many open tasks carry it, for the tag rows of the lists page.
struct TagInfo
{
    QString name;
    int openCount;

    TagInfo()
        : openCount(0)
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
    QString location;
    QString url;
    // The task's tags, held as CATEGORIES on the server.
    QStringList tags;
    QString status;
    QString completed;
    QString due;
    // UID of the parent task, empty when the task stands on its own.
    QString parentUid;
    int priority;
    int percent;
    // Minutes before the due time, or -1 when the task has no reminder.
    int reminder;
    // Reminder stamp already notified, so a reminder fires only once.
    QString remindedFor;
    bool dirty;
    bool deleted;

    LocalTask()
        : priority(0)
        , percent(0)
        , reminder(-1)
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
    QString location;
    QString url;
    QStringList tags;
    QString status;
    QString completed;
    QString start;
    QString due;
    QString parentUid;
    int priority;
    int percent;
    int reminder;

    TodoFields()
        : priority(0)
        , percent(0)
        , reminder(-1)
    {
    }
};

#endif
