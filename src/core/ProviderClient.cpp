#include "core/ProviderClient.h"

#include "core/EndpointValidator.h"
#include "core/PromptBuilder.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>

#include <algorithm>

namespace verbuno {

namespace {
constexpr qsizetype kMaximumBufferedResponse = 4 * 1024 * 1024;
constexpr int kPreferredP90LatencySeconds = 8;
constexpr int kPreferredP50Throughput = 20;

QString uiText(const char* source) {
    return QCoreApplication::translate("verbuno::ProviderClient", source);
}

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

int positiveTimeout(int value) {
    return std::max(value, 1);
}

int timeoutSeconds(int milliseconds) {
    return std::max(1, (milliseconds + 999) / 1000);
}

bool isRetryableFreeRoute(const QString& model) {
    const QString normalized = model.trimmed();
    return normalized.compare(QStringLiteral("openrouter/free"), Qt::CaseInsensitive) == 0 ||
           normalized.endsWith(QStringLiteral(":free"), Qt::CaseInsensitive);
}
} // namespace

ProviderClient::ProviderClient(QObject* parent)
    : ProviderClient(ProviderTimeouts{}, parent) {
}

ProviderClient::ProviderClient(const ProviderTimeouts& timeouts, QObject* parent)
    : QObject(parent)
    , m_timeouts(timeouts) {
    m_firstTokenTimer.setSingleShot(true);
    m_streamIdleTimer.setSingleShot(true);
    connect(&m_firstTokenTimer, &QTimer::timeout, this,
            &ProviderClient::handleFirstTokenTimeout);
    connect(&m_streamIdleTimer, &QTimer::timeout, this,
            &ProviderClient::handleStreamIdleTimeout);
}

ProviderClient::~ProviderClient() {
    m_firstTokenTimer.stop();
    m_streamIdleTimer.stop();
    if (m_translationReply) {
        m_translationReply->disconnect(this);
        m_translationReply->abort();
    }
    m_requestPayload.fill('\0');
    m_translationApiKey.fill(QChar('\0'));
}

void ProviderClient::translate(const TranslationRequest& request, const QString& apiKey) {
    const EndpointValidation validation = EndpointValidator::validate(request.provider.chatEndpoint);
    if (!validation.valid) {
        emit requestFailed(validation.error);
        return;
    }
    if (apiKey.trimmed().isEmpty()) {
        emit requestFailed(uiText("No API key is available for the selected provider."));
        return;
    }
    if (request.input.trimmed().isEmpty()) {
        emit requestFailed(uiText("Enter text to translate."));
        return;
    }
    if (request.provider.model.trimmed().isEmpty()) {
        emit requestFailed(uiText("Choose or enter a model identifier."));
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

    m_requestPayload = buildRequestPayload(request);
    m_translationApiKey = apiKey.trimmed();
    m_translationEndpoint = request.provider.chatEndpoint;
    m_translationUsesOpenRouter =
        request.provider.openRouter &&
        EndpointValidator::isOpenRouter(request.provider.chatEndpoint);
    m_retryableFreeRoute =
        m_translationUsesOpenRouter && isRetryableFreeRoute(request.provider.model);
    const int baseTimeout = m_retryableFreeRoute ? m_timeouts.freeRouteFirstTokenMs
                                                  : m_timeouts.firstTokenMs;
    const qsizetype extraSeconds =
        std::min<qsizetype>(30, request.input.size() / 2000);
    m_currentFirstTokenTimeoutMs =
        positiveTimeout(baseTimeout) + static_cast<int>(extraSeconds) * 1000;

    startTranslationAttempt();
    emit translationStarted();
}

QByteArray ProviderClient::buildRequestPayload(const TranslationRequest& request) {
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
        if (request.provider.preferFastProviders) {
            routing.insert(QStringLiteral("preferred_max_latency"),
                           QJsonObject{{QStringLiteral("p90"),
                                        kPreferredP90LatencySeconds}});
            routing.insert(QStringLiteral("preferred_min_throughput"),
                           QJsonObject{{QStringLiteral("p50"),
                                        kPreferredP50Throughput}});
        }
        body.insert(QStringLiteral("provider"), routing);
    }

    return QJsonDocument(body).toJson(QJsonDocument::Compact);
}

void ProviderClient::startTranslationAttempt() {
    resetAttemptState();
    ++m_attempt;

    QNetworkRequest networkRequest = makeRequest(
        m_translationEndpoint, m_translationApiKey, QByteArrayLiteral("text/event-stream"),
        std::max(m_timeouts.transferMs, m_currentFirstTokenTimeoutMs + 5000));
    if (m_translationUsesOpenRouter) {
        networkRequest.setRawHeader(QByteArrayLiteral("X-OpenRouter-Metadata"),
                                    QByteArrayLiteral("enabled"));
    }
    QNetworkReply* reply = m_network.post(networkRequest, m_requestPayload);
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
    m_firstTokenTimer.start(positiveTimeout(m_currentFirstTokenTimeoutMs));
}

void ProviderClient::retryFreeRoute() {
    if (!m_translationReply || !m_retryableFreeRoute || m_attempt != 1 ||
        m_receivedModelActivity) {
        return;
    }

    QNetworkReply* slowReply = m_translationReply;
    m_translationReply.clear();
    slowReply->disconnect(this);
    slowReply->abort();
    slowReply->deleteLater();
    if (!m_inferenceRoute.isEmpty()) {
        m_inferenceRoute = {};
        emit inferenceRouteResolved(m_inferenceRoute);
    }
    emit freeRouteRetrying();
    startTranslationAttempt();
}

void ProviderClient::handleFirstTokenTimeout() {
    if (!m_translationReply || m_receivedModelActivity) {
        return;
    }
    if (m_retryableFreeRoute && m_attempt == 1) {
        retryFreeRoute();
        return;
    }

    failTranslation(
        uiText("The model did not start responding within %1 seconds. Try again or choose a "
               "faster model.")
            .arg(timeoutSeconds(m_currentFirstTokenTimeoutMs)),
        m_translationReply);
}

void ProviderClient::handleStreamIdleTimeout() {
    if (!m_translationReply || !m_receivedModelActivity) {
        return;
    }
    const int seconds = timeoutSeconds(m_timeouts.streamIdleMs);
    const QString message =
        m_receivedContent
            ? uiText("The model stopped responding for %1 seconds. The partial translation was "
                     "kept; try again.")
                  .arg(seconds)
            : uiText("The model stopped responding for %1 seconds. Try again.").arg(seconds);
    failTranslation(message, m_translationReply);
}

void ProviderClient::fetchFreeModels(const ProviderSettings& provider, const QString& apiKey) {
    if (!provider.openRouter || !EndpointValidator::isOpenRouter(provider.modelsEndpoint)) {
        emit modelsFailed(
            uiText("Automatic model discovery is available for OpenRouter profiles only."));
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
        makeRequest(provider.modelsEndpoint, apiKey, QByteArrayLiteral("application/json"),
                    m_timeouts.modelCatalogMs);
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
    m_firstTokenTimer.stop();
    m_streamIdleTimer.stop();
    m_translationReply->abort();
}

bool ProviderClient::isTranslating() const {
    return !m_translationReply.isNull();
}

QNetworkRequest ProviderClient::makeRequest(const QUrl& endpoint,
                                            const QString& apiKey,
                                            const QByteArray& accept,
                                            int transferTimeoutMs) const {
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader(QByteArrayLiteral("Accept"), accept);
    if (!apiKey.isEmpty()) {
        request.setRawHeader(QByteArrayLiteral("Authorization"),
                             QByteArrayLiteral("Bearer ") + apiKey.toUtf8());
    }
    request.setRawHeader(QByteArrayLiteral("User-Agent"),
                         QByteArrayLiteral("Verbuno/") + QByteArrayLiteral(VERBUNO_VERSION));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);
    request.setTransferTimeout(positiveTimeout(transferTimeoutMs));
    return request;
}

void ProviderClient::handleTranslationData(QNetworkReply* reply) {
    const QByteArray bytes = reply->readAll();
    if (m_translationUsesOpenRouter && !m_receivedContent && !m_reportedRoutingWait &&
        bytes.contains(QByteArrayLiteral(": OPENROUTER PROCESSING"))) {
        m_reportedRoutingWait = true;
        emit openRouterStillRouting();
    }
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
    m_firstTokenTimer.stop();
    m_streamIdleTimer.stop();

    m_translationReply.clear();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const bool redirected = status >= 300 && status < 400;

    if (m_cancelled) {
        if (!m_failureEmitted) {
            emit requestFailed(uiText("Translation cancelled."));
        }
        resetTranslationState();
        return;
    }
    if (m_failureEmitted) {
        resetTranslationState();
        return;
    }
    if (redirected) {
        failTranslation(uiText(
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
    if (m_inferenceRoute.isEmpty()) {
        updateInferenceRoute(parseInferenceRoute(m_rawResponse));
    }
    if (m_accumulated.trimmed().isEmpty()) {
        failTranslation(m_receivedDone
                            ? uiText("The model returned an empty translation.")
                            : uiText("The provider returned an unsupported response."));
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
        emit modelsFailed(uiText("The model catalog redirected unexpectedly."));
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
        m_firstTokenTimer.stop();
        m_streamIdleTimer.stop();
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(event, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return;
    }
    const QJsonObject root = document.object();
    updateInferenceRoute(parseInferenceRoute(event));
    if (root.contains(QStringLiteral("error"))) {
        failTranslation(extractError(event, uiText("The provider reported a stream error.")),
                        reply);
        return;
    }

    const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        return;
    }
    const QJsonObject choice = choices.first().toObject();
    const QJsonObject delta = choice.value(QStringLiteral("delta")).toObject();
    const bool hasReasoning =
        !delta.value(QStringLiteral("reasoning")).toString().isEmpty() ||
        !delta.value(QStringLiteral("reasoning_content")).toString().isEmpty() ||
        !delta.value(QStringLiteral("reasoning_details")).toArray().isEmpty();
    if (hasReasoning) {
        const bool firstActivity = !m_receivedModelActivity;
        noteModelActivity();
        if (firstActivity) {
            emit modelProcessing();
        }
    }
    QString content = delta.value(QStringLiteral("content")).toString();
    if (content.isEmpty()) {
        content = choice.value(QStringLiteral("text")).toString();
    }
    appendContent(content);
}

void ProviderClient::noteModelActivity() {
    m_receivedModelActivity = true;
    m_firstTokenTimer.stop();
    m_streamIdleTimer.start(positiveTimeout(m_timeouts.streamIdleMs));
}

void ProviderClient::appendContent(const QString& content) {
    if (content.isEmpty()) {
        return;
    }
    noteModelActivity();
    m_receivedContent = true;
    m_accumulated.append(content);
    emit translationChunk(content);
}

void ProviderClient::updateInferenceRoute(const InferenceRoute& route) {
    InferenceRoute merged = m_inferenceRoute;
    if (!route.provider.isEmpty()) {
        merged.provider = route.provider;
    }
    if (!route.model.isEmpty()) {
        merged.model = route.model;
    }
    if (merged.provider == m_inferenceRoute.provider && merged.model == m_inferenceRoute.model) {
        return;
    }
    m_inferenceRoute = merged;
    emit inferenceRouteResolved(m_inferenceRoute);
}

void ProviderClient::failTranslation(const QString& message, QNetworkReply* reply) {
    if (m_failureEmitted) {
        return;
    }
    m_failureEmitted = true;
    m_firstTokenTimer.stop();
    m_streamIdleTimer.stop();
    emit requestFailed(sanitizedMessage(message));
    if (reply && reply->isRunning()) {
        reply->abort();
    }
}

void ProviderClient::resetAttemptState() {
    m_firstTokenTimer.stop();
    m_streamIdleTimer.stop();
    m_decoder.reset();
    m_rawResponse.fill('\0');
    m_rawResponse.clear();
    m_accumulated.clear();
    m_inferenceRoute = {};
    m_receivedModelActivity = false;
    m_receivedContent = false;
    m_reportedRoutingWait = false;
    m_cancelled = false;
    m_receivedDone = false;
    m_failureEmitted = false;
}

void ProviderClient::resetTranslationState() {
    resetAttemptState();
    m_requestPayload.fill('\0');
    m_requestPayload.clear();
    m_translationApiKey.fill(QChar('\0'));
    m_translationApiKey.clear();
    m_translationEndpoint.clear();
    m_currentFirstTokenTimeoutMs = 0;
    m_attempt = 0;
    m_translationUsesOpenRouter = false;
    m_retryableFreeRoute = false;
}

InferenceRoute ProviderClient::parseInferenceRoute(const QByteArray& payload) {
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }

    const QJsonObject root = document.object();
    InferenceRoute route;
    route.model = root.value(QStringLiteral("model")).toString().simplified().left(256);
    route.provider = root.value(QStringLiteral("provider")).toString().simplified().left(128);

    const QJsonObject metadata = root.value(QStringLiteral("openrouter_metadata")).toObject();
    const QJsonArray available = metadata.value(QStringLiteral("endpoints"))
                                     .toObject()
                                     .value(QStringLiteral("available"))
                                     .toArray();
    for (const QJsonValue& value : available) {
        const QJsonObject endpoint = value.toObject();
        if (!endpoint.value(QStringLiteral("selected")).toBool()) {
            continue;
        }
        route.provider =
            endpoint.value(QStringLiteral("provider")).toString().simplified().left(128);
        if (route.model.isEmpty()) {
            route.model = endpoint.value(QStringLiteral("model"))
                              .toString()
                              .simplified()
                              .left(256);
        }
        break;
    }

    if (route.provider.isEmpty()) {
        const QJsonArray attempts = metadata.value(QStringLiteral("attempts")).toArray();
        for (qsizetype index = attempts.size(); index > 0; --index) {
            const QJsonObject attempt = attempts.at(index - 1).toObject();
            const int status = attempt.value(QStringLiteral("status")).toInt();
            if (status < 200 || status >= 300) {
                continue;
            }
            route.provider =
                attempt.value(QStringLiteral("provider")).toString().simplified().left(128);
            if (route.model.isEmpty()) {
                route.model = attempt.value(QStringLiteral("model"))
                                  .toString()
                                  .simplified()
                                  .left(256);
            }
            break;
        }
    }
    return route;
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
    return sanitizedMessage(fallback.isEmpty() ? uiText("The provider request failed.")
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
        *error = uiText("The OpenRouter model catalog is not valid JSON.");
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
        *error = uiText("No free models were reported by OpenRouter.");
    }
    return result;
}

} // namespace verbuno
