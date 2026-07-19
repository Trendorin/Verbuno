#pragma once

#include "core/SseDecoder.h"
#include "core/TranslationTypes.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QPointer>

class QNetworkReply;

namespace translunix {

class ProviderClient final : public QObject {
    Q_OBJECT

public:
    explicit ProviderClient(QObject* parent = nullptr);

    void translate(const TranslationRequest& request, const QString& apiKey);
    void fetchFreeModels(const ProviderSettings& provider, const QString& apiKey = {});
    void cancel();

    [[nodiscard]] bool isTranslating() const;

signals:
    void translationStarted();
    void translationChunk(const QString& text);
    void translationFinished(const QString& text);
    void requestFailed(const QString& message);
    void modelsLoaded(const QVector<ModelInfo>& models);
    void modelsFailed(const QString& message);

private:
    [[nodiscard]] QNetworkRequest makeRequest(const QUrl& endpoint,
                                              const QString& apiKey,
                                              const QByteArray& accept) const;
    void handleTranslationData(QNetworkReply* reply);
    void handleTranslationFinished(QNetworkReply* reply);
    void handleModelsFinished(QNetworkReply* reply);
    void processSseEvent(const QByteArray& event, QNetworkReply* reply);
    void appendContent(const QString& content);
    void failTranslation(const QString& message, QNetworkReply* reply = nullptr);
    void resetTranslationState();

    [[nodiscard]] static QString extractCompletion(const QByteArray& payload);
    [[nodiscard]] static QString extractError(const QByteArray& payload,
                                              const QString& fallback);
    [[nodiscard]] static QString sanitizedMessage(const QString& message);
    [[nodiscard]] static QVector<ModelInfo> parseFreeModels(const QByteArray& payload,
                                                            QString* error);

    QNetworkAccessManager m_network;
    QPointer<QNetworkReply> m_translationReply;
    QPointer<QNetworkReply> m_modelsReply;
    SseDecoder m_decoder;
    QByteArray m_rawResponse;
    QString m_accumulated;
    bool m_cancelled = false;
    bool m_receivedDone = false;
    bool m_failureEmitted = false;
};

} // namespace translunix
