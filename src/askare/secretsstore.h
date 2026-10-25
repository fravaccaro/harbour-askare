#ifndef ASKARE_SECRETSSTORE_H
#define ASKARE_SECRETSSTORE_H

#include <functional>

#include <QObject>
#include <QString>

#include <Secrets/secret.h>
#include <Secrets/secretmanager.h>

class SecretsStore : public QObject
{
    Q_OBJECT

public:
    explicit SecretsStore(QObject *parent = nullptr);

    void load();
    void save(const QString &password);
    void remove();

signals:
    void loaded(const QString &password);
    void failed(const QString &error);

private:
    Sailfish::Secrets::Secret::Identifier identifier() const;
    void loadFromCollection();
    void ensureCollection(const std::function<void(bool)> &then);
    void writePassword(const QString &password, const std::function<void(bool)> &then);
    void storePassword(const QString &password, const std::function<void(bool)> &then);

    Sailfish::Secrets::SecretManager m_manager;
    bool m_collectionExists;
};

#endif
