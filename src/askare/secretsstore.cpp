#include "secretsstore.h"

#include <Secrets/collectionnamesrequest.h>
#include <Secrets/createcollectionrequest.h>
#include <Secrets/deletesecretrequest.h>
#include <Secrets/request.h>
#include <Secrets/result.h>
#include <Secrets/storedsecretrequest.h>
#include <Secrets/storesecretrequest.h>

namespace {

using namespace Sailfish::Secrets;

const QString CollectionName = QStringLiteral("askare");
const QString SecretName = QStringLiteral("app-password");

template <typename Request, typename Handler>
void run(Request *request, const Handler &handler)
{
    QObject::connect(request, &Sailfish::Secrets::Request::statusChanged,
                     request, [request, handler]() {
        if (request->status() != Sailfish::Secrets::Request::Finished)
            return;

        handler(request);
        request->deleteLater();
    });
    request->startRequest();
}

bool isMissing(const Result &result)
{
    switch (result.errorCode()) {
    case Result::InvalidSecretError:
    case Result::InvalidSecretIdentifierError:
    case Result::InvalidCollectionError:
        return true;
    default:
        return false;
    }
}

}

SecretsStore::SecretsStore(QObject *parent)
    : QObject(parent)
    , m_collectionExists(false)
{
}

Sailfish::Secrets::Secret::Identifier SecretsStore::identifier() const
{
    return Secret::Identifier(SecretName, CollectionName,
                              SecretManager::DefaultEncryptedStoragePluginName);
}

void SecretsStore::load()
{
    CollectionNamesRequest *request = new CollectionNamesRequest(this);
    request->setManager(&m_manager);
    request->setStoragePluginName(SecretManager::DefaultEncryptedStoragePluginName);
    run(request, [this](CollectionNamesRequest *request) {
        const Result result = request->result();
        if (result.code() != Result::Succeeded) {
            emit failed(result.errorMessage());
            return;
        }
        m_collectionExists = request->collectionNames().contains(CollectionName);
        if (m_collectionExists)
            loadFromCollection();
        else
            emit loaded(QString());
    });
}

void SecretsStore::loadFromCollection()
{
    StoredSecretRequest *request = new StoredSecretRequest(this);
    request->setManager(&m_manager);
    request->setIdentifier(identifier());
    request->setUserInteractionMode(SecretManager::SystemInteraction);
    run(request, [this](StoredSecretRequest *request) {
        const Result result = request->result();
        if (result.code() == Result::Succeeded) {
            emit loaded(QString::fromUtf8(request->secret().data()));
            return;
        }
        if (isMissing(result)) {
            emit loaded(QString());
            return;
        }
        emit failed(result.errorMessage());
    });
}

void SecretsStore::save(const QString &password)
{
    writePassword(password, [](bool) {});
}

void SecretsStore::writePassword(const QString &password, const std::function<void(bool)> &then)
{
    ensureCollection([this, password, then](bool ready) {
        if (!ready) {
            then(false);
            return;
        }
        DeleteSecretRequest *request = new DeleteSecretRequest(this);
        request->setManager(&m_manager);
        request->setIdentifier(identifier());
        request->setUserInteractionMode(SecretManager::SystemInteraction);
        run(request, [this, password, then](DeleteSecretRequest *) {
            storePassword(password, then);
        });
    });
}

void SecretsStore::storePassword(const QString &password, const std::function<void(bool)> &then)
{
    Secret secret(identifier());
    secret.setType(Secret::TypeBlob);
    secret.setData(password.toUtf8());
    StoreSecretRequest *request = new StoreSecretRequest(this);
    request->setManager(&m_manager);
    request->setSecretStorageType(StoreSecretRequest::CollectionSecret);
    request->setUserInteractionMode(SecretManager::SystemInteraction);
    request->setSecret(secret);
    run(request, [this, then](StoreSecretRequest *request) {
        const Result result = request->result();
        const bool stored = result.code() == Result::Succeeded;
        if (!stored)
            emit failed(result.errorMessage());
        then(stored);
    });
}

void SecretsStore::ensureCollection(const std::function<void(bool)> &then)
{
    if (m_collectionExists) {
        then(true);
        return;
    }
    CreateCollectionRequest *request = new CreateCollectionRequest(this);
    request->setManager(&m_manager);
    request->setCollectionName(CollectionName);
    request->setCollectionLockType(CreateCollectionRequest::DeviceLock);
    request->setDeviceLockUnlockSemantic(SecretManager::DeviceLockKeepUnlocked);
    request->setAccessControlMode(SecretManager::OwnerOnlyMode);
    request->setStoragePluginName(SecretManager::DefaultEncryptedStoragePluginName);
    request->setEncryptionPluginName(SecretManager::DefaultEncryptedStoragePluginName);
    run(request, [this, then](CreateCollectionRequest *request) {
        const Result result = request->result();
        const bool ready = result.code() == Result::Succeeded
                || result.errorCode() == Result::CollectionAlreadyExistsError;
        if (!ready) {
            emit failed(result.errorMessage());
            then(false);
            return;
        }
        m_collectionExists = true;
        then(true);
    });
}

void SecretsStore::remove()
{
    DeleteSecretRequest *request = new DeleteSecretRequest(this);
    request->setManager(&m_manager);
    request->setIdentifier(identifier());
    request->setUserInteractionMode(SecretManager::SystemInteraction);
    run(request, [](DeleteSecretRequest *) {});
}
