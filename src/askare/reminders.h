#ifndef ASKARE_REMINDERS_H
#define ASKARE_REMINDERS_H

#include <QObject>
#include <QString>
#include <QTimer>

class Store;

// Watches stored tasks and reports a reminder once its moment has passed.
// There is no background service: this runs while the application does, and
// while a reminder waits the window asks the system to wake it for a check.
class Reminders : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool armed READ armed NOTIFY armedChanged)

public:
    explicit Reminders(Store *store, QObject *parent = nullptr);

    // Whether a reminder is still ahead, and so worth being woken for.
    bool armed() const { return m_armed; }

    Q_INVOKABLE void check();

signals:
    void armedChanged();
    // parentSummary is empty unless the task is a subtask, where the title
    // alone often means little without it.
    void due(const QString &href, const QString &summary, const QString &dueText,
             const QString &parentSummary);

private:
    void setArmed(bool armed);

    Store *m_store;
    QTimer m_timer;
    bool m_armed;
};

#endif
