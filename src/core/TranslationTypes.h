#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QUrl>
#include <QVector>

namespace translunix {

enum class TranslationStyle {
    Natural = 0,
    Formal,
    Literal,
    Technical,
    Casual,
};

struct ProviderSettings {
    QString displayName = QStringLiteral("OpenRouter");
    QUrl chatEndpoint = QUrl(QStringLiteral("https://openrouter.ai/api/v1/chat/completions"));
    QUrl modelsEndpoint = QUrl(QStringLiteral("https://openrouter.ai/api/v1/models"));
    QString model = QStringLiteral("openrouter/free");
    bool openRouter = true;
    bool denyDataCollection = true;
    bool zeroDataRetention = false;
    bool preferThroughput = true;
};

struct TranslationRequest {
    QString input;
    QString sourceCode;
    QString sourceName;
    QString targetCode;
    QString targetName;
    TranslationStyle style = TranslationStyle::Natural;
    QString context;
    QString customInstruction;
    bool preserveFormatting = true;
    ProviderSettings provider;
};

struct TranslationRecord {
    QString id;
    QDateTime createdAt;
    QString sourceCode;
    QString targetCode;
    QString sourceText;
    QString translatedText;
    QString model;
};

struct ModelInfo {
    QString id;
    QString name;
    qint64 contextLength = 0;
    bool free = false;
};

} // namespace translunix

Q_DECLARE_METATYPE(translunix::ModelInfo)
Q_DECLARE_METATYPE(QVector<translunix::ModelInfo>)
