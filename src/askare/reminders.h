#ifndef ASKARE_REMINDERS_H
#define ASKARE_REMINDERS_H

#include <QObject>
#include <QString>
#include <QTimer>

class Store;

// Watches stored tasks and reports a reminder once its moment has passed.
// It only runs while the application does, so there is no background service.
class Reminders : public QObject
{
    Q_OBJECT

public:
    explicit Reminders(Store *store, QObject *parent = nullptr);

    Q_INVOKABLE void check();

signals:
    void due(const QString &href, const QString &summary, const QString &dueText);

private:
    Store *m_store;
    QTimer m_timer;
};

#endif
