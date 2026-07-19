#include "core/SecretStore.h"

#include <QTimer>
#include <QStringList>

#include <algorithm>

#if defined(TRANSLUNIX_HAS_KEYCHAIN)
#  if __has_include(<qt6keychain/keychain.h>)
#    include <qt6keychain/keychain.h>
#  elif __has_include(<keychain.h>)
#    include <keychain.h>
#  else
#    error "QtKeychain was found by CMake but its header is unavailable"
#  endif
#endif

namespace translunix {

namespace {
const QString kService = QStringLiteral("io.github.trendorin.TranslUnix");
}

SecretStore::SecretStore(QObject* parent)
    : QObject(parent) {
}

SecretStore::~SecretStore() {
    const QStringList accounts = m_sessionSecrets.keys();
    for (const QString& account : accounts) {
        eraseSessionSecret(account);
    }
}

void SecretStore::readSecret(const QString& account, bool allowPersistentRead) {
    const auto session = m_sessionSecrets.constFind(account);
    if (session != m_sessionSecrets.cend() && !session.value().isEmpty()) {
        const QString value = session.value();
        QTimer::singleShot(0, this, [this, account, value] { emit secretRead(account, value, {}); });
        return;
    }
    if (!allowPersistentRead) {
        QTimer::singleShot(0, this, [this, account] { emit secretRead(account, {}, {}); });
        return;
    }

#if defined(TRANSLUNIX_HAS_KEYCHAIN)
    auto* job = new QKeychain::ReadPasswordJob(kService, this);
    job->setKey(account);
    job->setInsecureFallback(false);
    connect(job, &QKeychain::Job::finished, this,
            [this, job, account](QKeychain::Job*) {
                if (job->error() == QKeychain::NoError) {
                    const QString secret = job->textData();
                    if (!secret.isEmpty()) {
                        m_sessionSecrets.insert(account, secret);
                    }
                    emit secretRead(account, secret, {});
                    return;
                }
                if (job->error() == QKeychain::EntryNotFound) {
                    emit secretRead(account, {}, {});
                    return;
                }
                emit secretRead(account, {}, job->errorString());
            });
    job->start();
#else
    QTimer::singleShot(0, this, [this, account] {
        emit secretRead(account, {},
                        QStringLiteral("This build has no secure keychain integration."));
    });
#endif
}

void SecretStore::storeSecret(const QString& account, const QString& secret, bool persist) {
    eraseSessionSecret(account);
    m_sessionSecrets.insert(account, secret);
    if (!persist) {
        QTimer::singleShot(0, this,
                           [this, account] { emit secretStored(account, false, {}); });
        return;
    }

#if defined(TRANSLUNIX_HAS_KEYCHAIN)
    auto* job = new QKeychain::WritePasswordJob(kService, this);
    job->setKey(account);
    job->setTextData(secret);
    job->setInsecureFallback(false);
    connect(job, &QKeychain::Job::finished, this,
            [this, job, account](QKeychain::Job*) {
                emit secretStored(account, job->error() == QKeychain::NoError,
                                  job->error() == QKeychain::NoError ? QString() : job->errorString());
            });
    job->start();
#else
    QTimer::singleShot(0, this, [this, account] {
        emit secretStored(account, false,
                          QStringLiteral("This build has no secure keychain integration."));
    });
#endif
}

void SecretStore::deleteSecret(const QString& account) {
    eraseSessionSecret(account);
#if defined(TRANSLUNIX_HAS_KEYCHAIN)
    auto* job = new QKeychain::DeletePasswordJob(kService, this);
    job->setKey(account);
    job->setInsecureFallback(false);
    connect(job, &QKeychain::Job::finished, this,
            [this, job, account](QKeychain::Job*) {
                const bool absent = job->error() == QKeychain::EntryNotFound;
                emit secretDeleted(account,
                                   job->error() == QKeychain::NoError || absent
                                       ? QString()
                                       : job->errorString());
            });
    job->start();
#else
    QTimer::singleShot(0, this, [this, account] { emit secretDeleted(account, {}); });
#endif
}

bool SecretStore::hasSessionSecret(const QString& account) const {
    const auto found = m_sessionSecrets.constFind(account);
    return found != m_sessionSecrets.cend() && !found.value().isEmpty();
}

bool SecretStore::secureStorageCompiled() const {
#if defined(TRANSLUNIX_HAS_KEYCHAIN)
    return true;
#else
    return false;
#endif
}

void SecretStore::eraseSessionSecret(const QString& account) {
    auto found = m_sessionSecrets.find(account);
    if (found == m_sessionSecrets.end()) {
        return;
    }
    std::fill(found.value().begin(), found.value().end(), QChar(u'\0'));
    m_sessionSecrets.erase(found);
}

} // namespace translunix
