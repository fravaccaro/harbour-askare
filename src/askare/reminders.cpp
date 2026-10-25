#include "reminders.h"

#include <QDateTime>
#include <QVector>

#include "ics.h"
#include "store.h"
#include "types.h"

namespace {

const int kCheckIntervalMs = 60 * 1000;

// The stamp identifies one reminder moment, so a changed due date or offset
// counts as a new reminder while an unchanged one stays silent.
QString reminderStamp(const QDateTime &moment)
{
    return moment.toString(QStringLiteral("yyyyMMddTHHmm"));
}

bool isPending(const LocalTask &task, const QDateTime &moment, const QDateTime &now)
{
    if (task.deleted || Ics::isCompleted(task.status))
        return false;
    if (!moment.isValid() || moment > now)
        return false;

    return task.remindedFor != reminderStamp(moment);
}

QString dueText(const LocalTask &task)
{
    const QString time = Ics::timeFromDue(task.due);
    const QString date = Ics::dateFromDue(task.due);
    return time.isEmpty() ? date : date + QLatin1Char(' ') + time;
}

}

Reminders::Reminders(Store *store, QObject *parent)
    : QObject(parent)
    , m_store(store)
{
    m_timer.setInterval(kCheckIntervalMs);
    connect(&m_timer, &QTimer::timeout, this, &Reminders::check);
    m_timer.start();
}

void Reminders::check()
{
    if (!m_store->isOpen())
        return;

    const QDateTime now = QDateTime::currentDateTime();
    const QVector<LocalTask> tasks = m_store->tasks();
    for (const LocalTask &task : tasks) {
        const QDateTime moment = Ics::reminderAt(task.due, task.reminder);
        if (!isPending(task, moment, now))
            continue;

        m_store->markReminded(task.href, reminderStamp(moment));
        emit due(task.href, task.summary, dueText(task));
    }
}
