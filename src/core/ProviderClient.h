#pragma once

#include "core/SseDecoder.h"
#include "core/TranslationTypes.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QPointer>
#include <QTimer>

class QNetworkReply;

namespace verbuno {

struct ProviderTimeouts {
    int firstTokenMs = 25000;
    int freeRouteFirstTokenMs = 12000;
    int streamIdleMs = 30000;
    int transferMs = 45000;
    int modelCatalogMs = 15000;
};

class ProviderClient final : public QObject {
    Q_OBJECT

public:
    explicit ProviderClient(QObject* parent = nullptr);
    ProviderClient(const ProviderTimeouts& timeouts, QObject* parent);
    ~ProviderClient() override;

    void translate(const TranslationRequest& request, const QString& apiKey);
    void fetchFreeModels(const ProviderSettings& provider, const QString& apiKey = {});
    void cancel();

    [[nodiscard]] bool isTranslating() const;
    [[nodiscard]] static InferenceRoute parseInferenceRoute(const QByteArray& payload);
    [[nodiscard]] static QByteArray buildRequestPayload(const TranslationRequest& request);

signals:
    void translationStarted();
    void openRouterStillRouting();
    void freeRouteRetrying();
    void modelProcessing();
    void translationChunk(const QString& text);
    void inferenceRouteResolved(const InferenceRoute& route);
    void translationFinished(const QString& text);
    void requestFailed(const QString& message);
    void modelsLoaded(const QVector<ModelInfo>& models);
    void modelsFailed(const QString& message);

private:
    [[nodiscard]] QNetworkRequest makeRequest(const QUrl& endpoint,
                                              const QString& apiKey,
                                              const QByteArray& accept,
                                              int transferTimeoutMs) const;
    void startTranslationAttempt();
    void retryFreeRoute();
    void handleFirstTokenTimeout();
    void handleStreamIdleTimeout();
    void handleTranslationData(QNetworkReply* reply);
    void handleTranslationFinished(QNetworkReply* reply);
    void handleModelsFinished(QNetworkReply* reply);
    void processSseEvent(const QByteArray& event, QNetworkReply* reply);
    void noteModelActivity();
    void appendContent(const QString& content);
    void updateInferenceRoute(const InferenceRoute& route);
    void failTranslation(const QString& message, QNetworkReply* reply = nullptr);
    void resetAttemptState();
    void resetTranslationState();

    [[nodiscard]] static QString extractCompletion(const QByteArray& payload);
    [[nodiscard]] static QString extractError(const QByteArray& payload,
                                              const QString& fallback);
    [[nodiscard]] static QString sanitizedMessage(const QString& message);
    [[nodiscard]] static QVector<ModelInfo> parseFreeModels(const QByteArray& payload,
                                                            QString* error);

    QNetworkAccessManager m_network;
    ProviderTimeouts m_timeouts;
    QPointer<QNetworkReply> m_translationReply;
    QPointer<QNetworkReply> m_modelsReply;
    QTimer m_firstTokenTimer;
    QTimer m_streamIdleTimer;
    SseDecoder m_decoder;
    QByteArray m_rawResponse;
    QByteArray m_requestPayload;
    QString m_accumulated;
    QString m_translationApiKey;
    QUrl m_translationEndpoint;
    InferenceRoute m_inferenceRoute;
    int m_currentFirstTokenTimeoutMs = 0;
    int m_attempt = 0;
    bool m_translationUsesOpenRouter = false;
    bool m_retryableFreeRoute = false;
    bool m_receivedModelActivity = false;
    bool m_receivedContent = false;
    bool m_reportedRoutingWait = false;
    bool m_cancelled = false;
    bool m_receivedDone = false;
    bool m_failureEmitted = false;
};

} // namespace verbuno
