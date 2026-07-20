#include "core/SecretStore.h"

#include <QTimer>
#include <QStringList>

#include <algorithm>

#if defined(VERBUNO_HAS_KEYCHAIN)
#  if __has_include(<qt6keychain/keychain.h>)
#    include <qt6keychain/keychain.h>
#  elif __has_include(<keychain.h>)
#    include <keychain.h>
#  else
#    error "QtKeychain was found by CMake but its header is unavailable"
#  endif
#endif

namespace verbuno {

namespace {
const QString kService = QStringLiteral("io.github.trendorin.Verbuno");
const QString kLegacyService = QStringLiteral("io.github.trendorin.TranslUnix");
constexpr int kKeychainReadTimeoutMs = 8000;
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

#if defined(VERBUNO_HAS_KEYCHAIN)
    const quint64 generation = ++m_nextReadGeneration;
    m_readGenerations.insert(account, generation);
    auto* job = new QKeychain::ReadPasswordJob(kService, this);
    job->setKey(account);
    job->setInsecureFallback(false);
    connect(job, &QKeychain::Job::finished, this,
            [this, job, account, generation](QKeychain::Job*) {
                if (m_readGenerations.value(account) != generation) {
                    return;
                }
                if (job->error() == QKeychain::NoError) {
                    m_readGenerations.remove(account);
                    const QString secret = job->textData();
                    if (!secret.isEmpty()) {
                        m_sessionSecrets.insert(account, secret);
                    }
                    emit secretRead(account, secret, {});
                    return;
                }
                if (job->error() == QKeychain::EntryNotFound) {
                    readLegacySecret(account, generation);
                    return;
                }
                m_readGenerations.remove(account);
                emit secretRead(account, {}, job->errorString());
            });
    job->start();
    QTimer::singleShot(kKeychainReadTimeoutMs, this, [this, account, generation] {
        if (m_readGenerations.value(account) != generation) {
            return;
        }
        m_readGenerations.remove(account);
        emit secretRead(
            account, {},
            tr("The system keychain did not respond within 8 seconds. Unlock it and try again."));
    });
#else
    QTimer::singleShot(0, this, [this, account] {
        emit secretRead(account, {},
                        tr("This build has no secure keychain integration."));
    });
#endif
}

void SecretStore::storeSecret(const QString& account, const QString& secret, bool persist) {
    const bool satisfiedPendingRead = m_readGenerations.remove(account) > 0;
    eraseSessionSecret(account);
    m_sessionSecrets.insert(account, secret);
    if (satisfiedPendingRead) {
        QTimer::singleShot(0, this,
                           [this, account, secret] { emit secretRead(account, secret, {}); });
    }
    if (!persist) {
        QTimer::singleShot(0, this,
                           [this, account] { emit secretStored(account, false, {}); });
        return;
    }

#if defined(VERBUNO_HAS_KEYCHAIN)
    auto* job = new QKeychain::WritePasswordJob(kService, this);
    job->setKey(account);
    job->setTextData(secret);
    job->setInsecureFallback(false);
    connect(job, &QKeychain::Job::finished, this,
            [this, job, account](QKeychain::Job*) {
                const bool stored = job->error() == QKeychain::NoError;
                if (stored) {
                    auto* cleanup = new QKeychain::DeletePasswordJob(kLegacyService, this);
                    cleanup->setKey(account);
                    cleanup->setInsecureFallback(false);
                    cleanup->start();
                }
                emit secretStored(account, stored,
                                  stored ? QString() : job->errorString());
            });
    job->start();
#else
    QTimer::singleShot(0, this, [this, account] {
        emit secretStored(account, false,
                          tr("This build has no secure keychain integration."));
    });
#endif
}

void SecretStore::deleteSecret(const QString& account) {
    const bool endedPendingRead = m_readGenerations.remove(account) > 0;
    eraseSessionSecret(account);
    if (endedPendingRead) {
        QTimer::singleShot(0, this, [this, account] { emit secretRead(account, {}, {}); });
    }
#if defined(VERBUNO_HAS_KEYCHAIN)
    auto* job = new QKeychain::DeletePasswordJob(kService, this);
    job->setKey(account);
    job->setInsecureFallback(false);
    connect(job, &QKeychain::Job::finished, this,
            [this, job, account](QKeychain::Job*) {
                const bool absent = job->error() == QKeychain::EntryNotFound;
                const QString error = job->error() == QKeychain::NoError || absent
                                          ? QString()
                                          : job->errorString();
                deleteLegacySecret(account, error);
            });
    job->start();
#else
    QTimer::singleShot(0, this, [this, account] { emit secretDeleted(account, {}); });
#endif
}

void SecretStore::readLegacySecret(const QString& account, quint64 generation) {
#if defined(VERBUNO_HAS_KEYCHAIN)
    auto* job = new QKeychain::ReadPasswordJob(kLegacyService, this);
    job->setKey(account);
    job->setInsecureFallback(false);
    connect(job, &QKeychain::Job::finished, this,
            [this, job, account, generation](QKeychain::Job*) {
                if (m_readGenerations.value(account) != generation) {
                    return;
                }
                if (job->error() == QKeychain::NoError) {
                    m_readGenerations.remove(account);
                    const QString secret = job->textData();
                    if (secret.isEmpty()) {
                        emit secretRead(account, {}, {});
                        return;
                    }
                    m_sessionSecrets.insert(account, secret);
                    emit secretRead(account, secret, {});

                    auto* migration = new QKeychain::WritePasswordJob(kService, this);
                    migration->setKey(account);
                    migration->setTextData(secret);
                    migration->setInsecureFallback(false);
                    connect(migration, &QKeychain::Job::finished, this,
                            [this, migration, account](QKeychain::Job*) {
                                if (migration->error() == QKeychain::NoError) {
                                    auto* cleanup =
                                        new QKeychain::DeletePasswordJob(kLegacyService, this);
                                    cleanup->setKey(account);
                                    cleanup->setInsecureFallback(false);
                                    cleanup->start();
                                }
                            });
                    migration->start();
                    return;
                }
                if (job->error() == QKeychain::EntryNotFound) {
                    m_readGenerations.remove(account);
                    emit secretRead(account, {}, {});
                    return;
                }
                m_readGenerations.remove(account);
                emit secretRead(account, {}, job->errorString());
            });
    job->start();
#else
    Q_UNUSED(account)
    Q_UNUSED(generation)
#endif
}

void SecretStore::deleteLegacySecret(const QString& account, const QString& precedingError) {
#if defined(VERBUNO_HAS_KEYCHAIN)
    auto* job = new QKeychain::DeletePasswordJob(kLegacyService, this);
    job->setKey(account);
    job->setInsecureFallback(false);
    connect(job, &QKeychain::Job::finished, this,
            [this, job, account, precedingError](QKeychain::Job*) {
                const bool absent = job->error() == QKeychain::EntryNotFound;
                const QString legacyError = job->error() == QKeychain::NoError || absent
                                                ? QString()
                                                : job->errorString();
                emit secretDeleted(account, !precedingError.isEmpty() ? precedingError
                                                                      : legacyError);
            });
    job->start();
#else
    Q_UNUSED(account)
    Q_UNUSED(precedingError)
#endif
}

bool SecretStore::hasSessionSecret(const QString& account) const {
    const auto found = m_sessionSecrets.constFind(account);
    return found != m_sessionSecrets.cend() && !found.value().isEmpty();
}

bool SecretStore::secureStorageCompiled() const {
#if defined(VERBUNO_HAS_KEYCHAIN)
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

} // namespace verbuno
