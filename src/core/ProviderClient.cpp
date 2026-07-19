#include "core/ProviderClient.h"

#include "core/EndpointValidator.h"
#include "core/PromptBuilder.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>

#include <algorithm>

namespace translunix {

namespace {
constexpr qsizetype kMaximumBufferedResponse = 4 * 1024 * 1024;

bool isZeroPrice(const QJsonValue& value, bool missingIsZero = false) {
    if (value.isUndefined() || value.isNull()) {
        return missingIsZero;
    }
    bool ok = false;
    const double price = value.isString() ? value.toString().toDouble(&ok) : value.toDouble();
    if (!value.isString()) {
        ok = value.isDouble();
    }
    return ok && price == 0.0;
}
} // namespace

ProviderClient::ProviderClient(QObject* parent)
    : QObject(parent) {
}

void ProviderClient::translate(const TranslationRequest& request, const QString& apiKey) {
    const EndpointValidation validation = EndpointValidator::validate(request.provider.chatEndpoint);
    if (!validation.valid) {
        emit requestFailed(validation.error);
        return;
    }
    if (apiKey.trimmed().isEmpty()) {
        emit requestFailed(QStringLiteral("No API key is available for the selected provider."));
        return;
    }
    if (request.input.trimmed().isEmpty()) {
        emit requestFailed(QStringLiteral("Enter text to translate."));
        return;
    }
    if (request.provider.model.trimmed().isEmpty()) {
        emit requestFailed(QStringLiteral("Choose or enter a model identifier."));
        return;
    }

    if (m_translationReply) {
        QNetworkReply* previous = m_translationReply;
        m_translationReply.clear();
        previous->disconnect(this);
        previous->abort();
        previous->deleteLater();
    }
    resetTranslationState();

    QJsonObject body;
    body.insert(QStringLiteral("model"), request.provider.model.trimmed());
    body.insert(QStringLiteral("stream"), true);
    body.insert(QStringLiteral("temperature"), 0.15);

    QJsonArray messages;
    messages.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("system")},
                                {QStringLiteral("content"), PromptBuilder::systemPrompt(request)}});
    messages.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("user")},
                                {QStringLiteral("content"), request.input}});
    body.insert(QStringLiteral("messages"), messages);

    if (request.provider.openRouter &&
        EndpointValidator::isOpenRouter(request.provider.chatEndpoint)) {
        QJsonObject routing;
        routing.insert(QStringLiteral("allow_fallbacks"), true);
        if (request.provider.denyDataCollection) {
            routing.insert(QStringLiteral("data_collection"), QStringLiteral("deny"));
        }
        if (request.provider.zeroDataRetention) {
            routing.insert(QStringLiteral("zdr"), true);
        }
        if (request.provider.preferThroughput) {
            routing.insert(QStringLiteral("sort"), QStringLiteral("throughput"));
        }
        body.insert(QStringLiteral("provider"), routing);
    }

    QNetworkRequest networkRequest =
        makeRequest(request.provider.chatEndpoint, apiKey, QByteArrayLiteral("text/event-stream"));
    QNetworkReply* reply = m_network.post(networkRequest, QJsonDocument(body).toJson(QJsonDocument::Compact));
    m_translationReply = reply;

    connect(reply, &QNetworkReply::readyRead, this, [this, reply] {
        if (reply == m_translationReply) {
            handleTranslationData(reply);
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        handleTranslationFinished(reply);
        reply->deleteLater();
    });
    emit translationStarted();
}

void ProviderClient::fetchFreeModels(const ProviderSettings& provider, const QString& apiKey) {
    if (!provider.openRouter || !EndpointValidator::isOpenRouter(provider.modelsEndpoint)) {
        emit modelsFailed(
            QStringLiteral("Automatic model discovery is available for OpenRouter profiles only."));
        return;
    }
    const EndpointValidation validation = EndpointValidator::validate(provider.modelsEndpoint);
    if (!validation.valid) {
        emit modelsFailed(validation.error);
        return;
    }

    if (m_modelsReply) {
        QNetworkReply* previous = m_modelsReply;
        m_modelsReply.clear();
        previous->disconnect(this);
        previous->abort();
        previous->deleteLater();
    }

    QNetworkRequest request =
        makeRequest(provider.modelsEndpoint, apiKey, QByteArrayLiteral("application/json"));
    QNetworkReply* reply = m_network.get(request);
    m_modelsReply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        handleModelsFinished(reply);
        reply->deleteLater();
    });
}

void ProviderClient::cancel() {
    if (!m_translationReply) {
        return;
    }
    m_cancelled = true;
    m_translationReply->abort();
}

bool ProviderClient::isTranslating() const {
    return !m_translationReply.isNull();
}

QNetworkRequest ProviderClient::makeRequest(const QUrl& endpoint,
                                            const QString& apiKey,
                                            const QByteArray& accept) const {
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader(QByteArrayLiteral("Accept"), accept);
    if (!apiKey.isEmpty()) {
        request.setRawHeader(QByteArrayLiteral("Authorization"),
                             QByteArrayLiteral("Bearer ") + apiKey.toUtf8());
    }
    request.setRawHeader(QByteArrayLiteral("User-Agent"),
                         QByteArrayLiteral("TranslUnix/") + QByteArrayLiteral(TRANSLUNIX_VERSION));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);
    request.setTransferTimeout(90000);
    return request;
}

void ProviderClient::handleTranslationData(QNetworkReply* reply) {
    const QByteArray bytes = reply->readAll();
    if (m_rawResponse.size() + bytes.size() <= kMaximumBufferedResponse) {
        m_rawResponse.append(bytes);
    }
    const QVector<QByteArray> events = m_decoder.feed(bytes);
    for (const QByteArray& event : events) {
        processSseEvent(event, reply);
        if (m_failureEmitted) {
            return;
        }
    }
}

void ProviderClient::handleTranslationFinished(QNetworkReply* reply) {
    if (reply != m_translationReply) {
        return;
    }
    handleTranslationData(reply);
    const QVector<QByteArray> trailing = m_decoder.finish();
    for (const QByteArray& event : trailing) {
        processSseEvent(event, reply);
    }

    m_translationReply.clear();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const bool redirected = status >= 300 && status < 400;

    if (m_cancelled) {
        if (!m_failureEmitted) {
            emit requestFailed(QStringLiteral("Translation cancelled."));
        }
        resetTranslationState();
        return;
    }
    if (m_failureEmitted) {
        resetTranslationState();
        return;
    }
    if (redirected) {
        failTranslation(QStringLiteral(
            "The provider redirected the request. Redirects are refused to protect the API key."));
        resetTranslationState();
        return;
    }
    if (reply->error() != QNetworkReply::NoError || status < 200 || status >= 300) {
        failTranslation(extractError(m_rawResponse, reply->errorString()));
        resetTranslationState();
        return;
    }

    if (m_accumulated.isEmpty()) {
        m_accumulated = extractCompletion(m_rawResponse);
    }
    if (m_accumulated.trimmed().isEmpty()) {
        failTranslation(m_receivedDone
                            ? QStringLiteral("The model returned an empty translation.")
                            : QStringLiteral("The provider returned an unsupported response."));
        resetTranslationState();
        return;
    }

    const QString result = m_accumulated;
    resetTranslationState();
    emit translationFinished(result);
}

void ProviderClient::handleModelsFinished(QNetworkReply* reply) {
    if (reply != m_modelsReply) {
        return;
    }
    m_modelsReply.clear();
    const QByteArray payload = reply->readAll();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status >= 300 && status < 400) {
        emit modelsFailed(QStringLiteral("The model catalog redirected unexpectedly."));
        return;
    }
    if (reply->error() != QNetworkReply::NoError || status < 200 || status >= 300) {
        emit modelsFailed(extractError(payload, reply->errorString()));
        return;
    }

    QString error;
    const QVector<ModelInfo> models = parseFreeModels(payload, &error);
    if (!error.isEmpty()) {
        emit modelsFailed(error);
        return;
    }
    emit modelsLoaded(models);
}

void ProviderClient::processSseEvent(const QByteArray& event, QNetworkReply* reply) {
    if (event.trimmed() == QByteArrayLiteral("[DONE]")) {
        m_receivedDone = true;
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(event, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return;
    }
    const QJsonObject root = document.object();
    if (root.contains(QStringLiteral("error"))) {
        failTranslation(extractError(event, QStringLiteral("The provider reported a stream error.")),
                        reply);
        return;
    }

    const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        return;
    }
    const QJsonObject choice = choices.first().toObject();
    const QJsonObject delta = choice.value(QStringLiteral("delta")).toObject();
    QString content = delta.value(QStringLiteral("content")).toString();
    if (content.isEmpty()) {
        content = choice.value(QStringLiteral("text")).toString();
    }
    appendContent(content);
}

void ProviderClient::appendContent(const QString& content) {
    if (content.isEmpty()) {
        return;
    }
    m_accumulated.append(content);
    emit translationChunk(content);
}

void ProviderClient::failTranslation(const QString& message, QNetworkReply* reply) {
    if (m_failureEmitted) {
        return;
    }
    m_failureEmitted = true;
    emit requestFailed(sanitizedMessage(message));
    if (reply && reply->isRunning()) {
        reply->abort();
    }
}

void ProviderClient::resetTranslationState() {
    m_decoder.reset();
    m_rawResponse.fill('\0');
    m_rawResponse.clear();
    m_accumulated.clear();
    m_cancelled = false;
    m_receivedDone = false;
    m_failureEmitted = false;
}

QString ProviderClient::extractCompletion(const QByteArray& payload) {
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }
    const QJsonArray choices = document.object().value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        return {};
    }
    const QJsonObject choice = choices.first().toObject();
    const QJsonObject message = choice.value(QStringLiteral("message")).toObject();
    QString content = message.value(QStringLiteral("content")).toString();
    if (content.isEmpty()) {
        content = choice.value(QStringLiteral("text")).toString();
    }
    return content;
}

QString ProviderClient::extractError(const QByteArray& payload, const QString& fallback) {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error == QJsonParseError::NoError && document.isObject()) {
        const QJsonValue errorValue = document.object().value(QStringLiteral("error"));
        if (errorValue.isObject()) {
            const QString message = errorValue.toObject().value(QStringLiteral("message")).toString();
            if (!message.isEmpty()) {
                return sanitizedMessage(message);
            }
        } else if (errorValue.isString()) {
            return sanitizedMessage(errorValue.toString());
        }
    }
    return sanitizedMessage(fallback.isEmpty() ? QStringLiteral("The provider request failed.")
                                               : fallback);
}

QString ProviderClient::sanitizedMessage(const QString& message) {
    QString result = message.left(600).simplified();
    static const QRegularExpression tokenPattern(
        QStringLiteral(R"((?:sk|key|token)[-_][A-Za-z0-9_-]{8,})"),
        QRegularExpression::CaseInsensitiveOption);
    result.replace(tokenPattern, QStringLiteral("[credential redacted]"));
    return result;
}

QVector<ModelInfo> ProviderClient::parseFreeModels(const QByteArray& payload, QString* error) {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        *error = QStringLiteral("The OpenRouter model catalog is not valid JSON.");
        return {};
    }

    QVector<ModelInfo> result;
    const QJsonArray data = document.object().value(QStringLiteral("data")).toArray();
    for (const QJsonValue& value : data) {
        const QJsonObject model = value.toObject();
        const QString id = model.value(QStringLiteral("id")).toString().trimmed();
        if (id.isEmpty()) {
            continue;
        }
        const QJsonObject pricing = model.value(QStringLiteral("pricing")).toObject();
        const bool free = id.endsWith(QStringLiteral(":free")) ||
                          (isZeroPrice(pricing.value(QStringLiteral("prompt"))) &&
                           isZeroPrice(pricing.value(QStringLiteral("completion"))) &&
                           isZeroPrice(pricing.value(QStringLiteral("request")), true));
        if (!free) {
            continue;
        }

        ModelInfo info;
        info.id = id;
        info.name = model.value(QStringLiteral("name")).toString(id);
        info.contextLength =
            static_cast<qint64>(model.value(QStringLiteral("context_length")).toDouble());
        info.free = true;
        result.push_back(info);
    }

    const auto router = std::find_if(result.cbegin(), result.cend(), [](const ModelInfo& model) {
        return model.id == QStringLiteral("openrouter/free");
    });
    if (router == result.cend()) {
        result.push_back({QStringLiteral("openrouter/free"),
                          QStringLiteral("OpenRouter Free Models Router"), 0, true});
    }
    std::sort(result.begin(), result.end(), [](const ModelInfo& left, const ModelInfo& right) {
        if (left.id == right.id) {
            return false;
        }
        if (left.id == QStringLiteral("openrouter/free")) {
            return true;
        }
        if (right.id == QStringLiteral("openrouter/free")) {
            return false;
        }
        return QString::localeAwareCompare(left.name, right.name) < 0;
    });
    if (result.isEmpty()) {
        *error = QStringLiteral("No free models were reported by OpenRouter.");
    }
    return result;
}

} // namespace translunix
