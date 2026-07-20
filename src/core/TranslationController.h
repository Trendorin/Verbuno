#pragma once

#include "core/TranslationTypes.h"

#include <QObject>

#include <optional>

namespace verbuno {

class AppSettings;
class HistoryStore;
class ProviderClient;
class SecretStore;

class TranslationController final : public QObject {
    Q_OBJECT

public:
    TranslationController(AppSettings* settings,
                          SecretStore* secretStore,
                          HistoryStore* history,
                          QObject* parent = nullptr);

    void translate(const QString& input,
                   const QString& sourceCode,
                   const QString& targetCode);
    void cancel();

    void saveApiKey(const QString& apiKey, bool persist);
    void clearApiKey();
    void checkApiKeyAvailability();
    void refreshFreeModels();

    [[nodiscard]] bool isBusy() const;
    [[nodiscard]] QString credentialAccount() const;
    [[nodiscard]] InferenceRoute inferenceRoute() const;
    [[nodiscard]] QString requestedModel() const;
    [[nodiscard]] bool inferenceRouteMatchesCurrentProvider() const;
    [[nodiscard]] const QVector<TranslationRecord>& historyRecords() const;
    void clearHistory();

signals:
    void requestStarted();
    void translationChunk(const QString& text);
    void inferenceRouteChanged(const InferenceRoute& route);
    void translationFinished(const QString& text);
    void requestFailed(const QString& message);
    void modelsLoaded(const QVector<ModelInfo>& models);
    void modelsFailed(const QString& message);
    void apiKeyStored(bool persistent, const QString& error);
    void apiKeyDeleted(const QString& error);
    void apiKeyAvailabilityChanged(bool available, const QString& error);
    void historyChanged();

private:
    enum class SecretPurpose { None, Translation, Models, AvailabilityCheck };

    void applyHistoryPolicy();
    void handleSecretRead(const QString& account, const QString& secret, const QString& error);
    [[nodiscard]] std::optional<TranslationRequest>
    buildRequest(const QString& input,
                 const QString& sourceCode,
                 const QString& targetCode,
                 QString* error) const;
    [[nodiscard]] static QString accountForProvider(const ProviderSettings& provider);

    AppSettings* m_settings;
    SecretStore* m_secretStore;
    HistoryStore* m_history;
    ProviderClient* m_client;
    SecretPurpose m_secretPurpose = SecretPurpose::None;
    std::optional<TranslationRequest> m_pendingRequest;
    std::optional<TranslationRequest> m_activeRequest;
    InferenceRoute m_inferenceRoute;
    QString m_routeAccount;
    QString m_requestedModel;
};

} // namespace verbuno
