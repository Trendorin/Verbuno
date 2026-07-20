#pragma once

#include <QHash>
#include <QObject>
#include <QString>

namespace verbuno {

class SecretStore final : public QObject {
    Q_OBJECT

public:
    explicit SecretStore(QObject* parent = nullptr);
    ~SecretStore() override;

    void readSecret(const QString& account, bool allowPersistentRead);
    void storeSecret(const QString& account, const QString& secret, bool persist);
    void deleteSecret(const QString& account);

    [[nodiscard]] bool hasSessionSecret(const QString& account) const;
    [[nodiscard]] bool secureStorageCompiled() const;

signals:
    void secretRead(const QString& account, const QString& secret, const QString& error);
    void secretStored(const QString& account, bool persistent, const QString& error);
    void secretDeleted(const QString& account, const QString& error);

private:
    void eraseSessionSecret(const QString& account);
    void readLegacySecret(const QString& account, quint64 generation);
    void deleteLegacySecret(const QString& account, const QString& precedingError = {});

    QHash<QString, QString> m_sessionSecrets;
    QHash<QString, quint64> m_readGenerations;
    quint64 m_nextReadGeneration = 0;
};

} // namespace verbuno
