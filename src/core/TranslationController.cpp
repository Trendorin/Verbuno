#include "core/TranslationController.h"

#include "core/AppSettings.h"
#include "core/EndpointValidator.h"
#include "core/HistoryStore.h"
#include "core/LanguageCatalog.h"
#include "core/ProviderClient.h"
#include "core/SecretStore.h"

#include <QCryptographicHash>

#include <utility>

namespace verbuno {

TranslationController::TranslationController(AppSettings* settings,
                                             SecretStore* secretStore,
                                             HistoryStore* history,
                                             QObject* parent)
    : QObject(parent)
    , m_settings(settings)
    , m_secretStore(secretStore)
    , m_history(history)
    , m_client(new ProviderClient(this)) {
    applyHistoryPolicy();
    connect(m_settings, &AppSettings::changed, this,
            &TranslationController::applyHistoryPolicy);
    connect(m_secretStore, &SecretStore::secretRead, this,
            &TranslationController::handleSecretRead);
    connect(m_secretStore, &SecretStore::secretStored, this,
            [this](const QString& account, bool persistent, const QString& error) {
                if (account == credentialAccount()) {
                    if (error.isEmpty()) {
                        m_settings->setRememberApiKey(persistent);
                    }
                    emit apiKeyStored(persistent, error);
                    if (error.isEmpty()) {
                        emit apiKeyAvailabilityChanged(true, {});
                    }
                }
            });
    connect(m_secretStore, &SecretStore::secretDeleted, this,
            [this](const QString& account, const QString& error) {
                if (account == credentialAccount()) {
                    emit apiKeyDeleted(error);
                    if (error.isEmpty()) {
                        emit apiKeyAvailabilityChanged(false, {});
                    }
                }
            });
    connect(m_client, &ProviderClient::translationStarted, this, [this] {
        emit inferenceRouteChanged(m_inferenceRoute);
        emit requestStarted();
    });
    connect(m_client, &ProviderClient::translationChunk, this,
            &TranslationController::translationChunk);
    connect(m_client, &ProviderClient::inferenceRouteResolved, this,
            [this](const InferenceRoute& route) {
                m_inferenceRoute = route;
                emit inferenceRouteChanged(m_inferenceRoute);
            });
    connect(m_client, &ProviderClient::translationFinished, this,
            [this](const QString& output) {
                if (m_activeRequest.has_value()) {
                    TranslationRecord record;
                    record.createdAt = QDateTime::currentDateTimeUtc();
                    record.sourceCode = m_activeRequest->sourceCode;
                    record.targetCode = m_activeRequest->targetCode;
                    record.sourceText = m_activeRequest->input;
                    record.translatedText = output;
                    record.model = m_inferenceRoute.model.isEmpty()
                                       ? m_activeRequest->provider.model
                                       : m_inferenceRoute.model;
                    m_history->append(std::move(record));
                }
                m_activeRequest.reset();
                emit translationFinished(output);
            });
    connect(m_client, &ProviderClient::requestFailed, this, [this](const QString& message) {
        m_activeRequest.reset();
        emit requestFailed(message);
    });
    connect(m_client, &ProviderClient::modelsLoaded, this, &TranslationController::modelsLoaded);
    connect(m_client, &ProviderClient::modelsFailed, this, &TranslationController::modelsFailed);
    connect(m_history, &HistoryStore::changed, this, &TranslationController::historyChanged);
    connect(m_history, &HistoryStore::storeError, this, [this](const QString& error) {
        emit requestFailed(error);
    });
}

void TranslationController::translate(const QString& input,
                                      const QString& sourceCode,
                                      const QString& targetCode) {
    if (isBusy() || m_secretPurpose != SecretPurpose::None) {
        emit requestFailed(tr("A provider request is already running."));
        return;
    }
    QString error;
    m_pendingRequest = buildRequest(input, sourceCode, targetCode, &error);
    if (!m_pendingRequest.has_value()) {
        emit requestFailed(error);
        return;
    }

    m_secretPurpose = SecretPurpose::Translation;
    m_secretStore->readSecret(credentialAccount(), m_settings->rememberApiKey());
}

void TranslationController::cancel() {
    m_pendingRequest.reset();
    m_secretPurpose = SecretPurpose::None;
    m_client->cancel();
}

void TranslationController::saveApiKey(const QString& apiKey, bool persist) {
    const QString normalized = apiKey.trimmed();
    if (normalized.size() < 8) {
        emit apiKeyStored(false, tr("The API key is unexpectedly short."));
        return;
    }
    m_secretStore->storeSecret(credentialAccount(), normalized, persist);
}

void TranslationController::clearApiKey() {
    m_secretStore->deleteSecret(credentialAccount());
}

void TranslationController::checkApiKeyAvailability() {
    if (isBusy() || m_secretPurpose != SecretPurpose::None) {
        emit apiKeyAvailabilityChanged(false,
                                       tr("Wait for the current provider request to finish."));
        return;
    }
    m_secretPurpose = SecretPurpose::AvailabilityCheck;
    m_secretStore->readSecret(credentialAccount(), m_settings->rememberApiKey());
}

void TranslationController::refreshFreeModels() {
    if (isBusy() || m_secretPurpose != SecretPurpose::None) {
        emit modelsFailed(tr("Wait for the current provider request to finish."));
        return;
    }
    m_secretPurpose = SecretPurpose::Models;
    m_secretStore->readSecret(credentialAccount(), m_settings->rememberApiKey());
}

bool TranslationController::isBusy() const {
    return m_client->isTranslating();
}

QString TranslationController::credentialAccount() const {
    return accountForProvider(m_settings->provider());
}

InferenceRoute TranslationController::inferenceRoute() const {
    return m_inferenceRoute;
}

QString TranslationController::requestedModel() const {
    return m_requestedModel;
}

bool TranslationController::inferenceRouteMatchesCurrentProvider() const {
    const ProviderSettings provider = m_settings->provider();
    return !m_routeAccount.isEmpty() && m_routeAccount == accountForProvider(provider) &&
           m_requestedModel == provider.model.trimmed();
}

const QVector<TranslationRecord>& TranslationController::historyRecords() const {
    return m_history->records();
}

void TranslationController::clearHistory() {
    m_history->clear();
}

void TranslationController::applyHistoryPolicy() {
    m_history->setPolicy(m_settings->historyLimit(), m_settings->historyRetentionDays());
    m_history->setEnabled(m_settings->historyEnabled());
}

void TranslationController::handleSecretRead(const QString& account,
                                             const QString& secret,
                                             const QString& error) {
    if (account != credentialAccount() || m_secretPurpose == SecretPurpose::None) {
        return;
    }
    const SecretPurpose purpose = m_secretPurpose;
    m_secretPurpose = SecretPurpose::None;

    if (purpose == SecretPurpose::AvailabilityCheck) {
        emit apiKeyAvailabilityChanged(!secret.isEmpty(), error);
        return;
    }

    if (purpose == SecretPurpose::Models) {
        if (!error.isEmpty()) {
            emit modelsFailed(error);
            return;
        }
        m_client->fetchFreeModels(m_settings->provider(), secret);
        return;
    }

    if (!error.isEmpty()) {
        m_pendingRequest.reset();
        emit requestFailed(error);
        return;
    }
    if (secret.isEmpty()) {
        m_pendingRequest.reset();
        emit requestFailed(tr("Add an API key in Settings before translating."));
        return;
    }
    if (!m_pendingRequest.has_value()) {
        return;
    }

    m_activeRequest = std::move(m_pendingRequest);
    m_pendingRequest.reset();
    m_inferenceRoute = {};
    m_routeAccount = accountForProvider(m_activeRequest->provider);
    m_requestedModel = m_activeRequest->provider.model.trimmed();
    m_client->translate(*m_activeRequest, secret);
}

std::optional<TranslationRequest>
TranslationController::buildRequest(const QString& input,
                                    const QString& sourceCode,
                                    const QString& targetCode,
                                    QString* error) const {
    if (input.trimmed().isEmpty()) {
        *error = tr("Enter text to translate.");
        return std::nullopt;
    }
    if (input.size() > m_settings->maximumInputCharacters()) {
        *error = tr("The text exceeds the configured input limit.");
        return std::nullopt;
    }
    const std::optional<Language> target = LanguageCatalog::byCode(targetCode);
    if (!target.has_value()) {
        *error = tr("Choose a valid target language.");
        return std::nullopt;
    }

    QString sourceName = QStringLiteral("Automatic detection");
    if (sourceCode != QStringLiteral("auto")) {
        const std::optional<Language> source = LanguageCatalog::byCode(sourceCode);
        if (!source.has_value()) {
            *error = tr("Choose a valid source language.");
            return std::nullopt;
        }
        if (source->code.compare(target->code, Qt::CaseInsensitive) == 0) {
            *error = tr("Source and target languages must be different.");
            return std::nullopt;
        }
        sourceName = source->name;
    }

    const ProviderSettings provider = m_settings->provider();
    const EndpointValidation validation = EndpointValidator::validate(provider.chatEndpoint);
    if (!validation.valid) {
        *error = validation.error;
        return std::nullopt;
    }

    TranslationRequest request;
    request.input = input;
    request.sourceCode = sourceCode;
    request.sourceName = sourceName;
    request.targetCode = target->code;
    request.targetName = target->name;
    request.style = m_settings->translationStyle();
    request.customInstruction = m_settings->customInstruction();
    request.preserveFormatting = m_settings->preserveFormatting();
    request.provider = provider;
    return request;
}

QString TranslationController::accountForProvider(const ProviderSettings& provider) {
    const QByteArray identity =
        provider.displayName.toUtf8() + '\n' + provider.chatEndpoint.toEncoded(QUrl::FullyEncoded);
    const QByteArray digest = QCryptographicHash::hash(identity, QCryptographicHash::Sha256).toHex();
    return QStringLiteral("provider-%1").arg(QString::fromLatin1(digest.left(24)));
}

} // namespace verbuno
