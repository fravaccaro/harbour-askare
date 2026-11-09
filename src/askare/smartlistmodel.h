#ifndef ASKARE_SMARTLISTMODEL_H
#define ASKARE_SMARTLISTMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVariantMap>
#include <QVector>

class Config;
class Store;
class TaskModel;

// The saved searches as a list the lists page binds to, each with the number
// of tasks it keeps. The count is worked out against the store as it stands
// rather than stored beside the filters, so it follows every edit and sync.
class SmartListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        CountRole,
        FiltersRole
    };

    explicit SmartListModel(Config *config, TaskModel *tasks, Store *store,
                            QObject *parent = nullptr);

    int count() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void reload();

signals:
    void countChanged();

private:
    QVector<QVariantMap> computeItems() const;
    // Brings m_items to \a next in place, by insert, move and remove, so a
    // delegate - and the remorse a row being deleted keeps inside it - is not
    // destroyed by a reset on every reload.
    void syncItems(const QVector<QVariantMap> &next);
    int indexOfId(int id, int from) const;

    Config *m_config;
    TaskModel *m_tasks;
    Store *m_store;
    QVector<QVariantMap> m_items;
};

#endif
