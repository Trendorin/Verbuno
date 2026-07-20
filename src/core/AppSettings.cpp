#include "core/AppSettings.h"

#include "core/InterfaceLanguageManager.h"

#include <QFile>
#include <QFileInfo>

#include <algorithm>

namespace verbuno {

namespace {
constexpr auto kOpenRouterChat = "https://openrouter.ai/api/v1/chat/completions";
constexpr auto kOpenRouterModels = "https://openrouter.ai/api/v1/models";
constexpr auto kDefaultModel = "openrouter/free";
constexpr auto kLegacyMigration = "migration/translunix-0.1";
constexpr auto kStorageSchema = "storage/schemaVersion";
constexpr int kCurrentStorageSchema = 1;
} // namespace

AppSettings::AppSettings(QObject* parent)
    : QObject(parent)
    , m_settings(QStringLiteral("Trendorin"), QStringLiteral("Verbuno")) {
    m_settings.setAtomicSyncRequired(true);
    const int previousStorageSchema =
        m_settings.value(QString::fromLatin1(kStorageSchema), 0).toInt();
    if (!m_settings.contains(QString::fromLatin1(kLegacyMigration))) {
        QSettings legacy(QStringLiteral("Trendorin"), QStringLiteral("TranslUnix"));
        for (const QString& key : legacy.allKeys()) {
            if (!m_settings.contains(key)) {
                m_settings.setValue(key, legacy.value(key));
            }
        }
        m_settings.setValue(QString::fromLatin1(kLegacyMigration), true);
    }
    if (previousStorageSchema < 1) {
        // Earlier releases silently defaulted to session-only keys. The new persistent choice
        // changes only the next explicit key save; it never copies a secret into normal settings.
        m_settings.setValue(QStringLiteral("privacy/rememberApiKey"), true);
    }
    m_settings.setValue(QString::fromLatin1(kStorageSchema), kCurrentStorageSchema);
    sync();
}

ProviderSettings AppSettings::provider() const {
    ProviderSettings value;
    value.displayName = m_settings.value(QStringLiteral("provider/name"), QStringLiteral("OpenRouter"))
                            .toString();
    value.chatEndpoint = QUrl(m_settings.value(QStringLiteral("provider/chatEndpoint"),
                                               QString::fromLatin1(kOpenRouterChat))
                                  .toString());
    value.modelsEndpoint = QUrl(m_settings.value(QStringLiteral("provider/modelsEndpoint"),
                                                 QString::fromLatin1(kOpenRouterModels))
                                    .toString());
    value.model =
        m_settings.value(QStringLiteral("provider/model"), QString::fromLatin1(kDefaultModel))
            .toString();
    value.openRouter = m_settings.value(QStringLiteral("provider/openRouter"), true).toBool();
    value.denyDataCollection =
        m_settings.value(QStringLiteral("privacy/denyDataCollection"), true).toBool();
    value.zeroDataRetention =
        m_settings.value(QStringLiteral("privacy/zeroDataRetention"), false).toBool();
    value.preferThroughput =
        m_settings.value(QStringLiteral("provider/preferThroughput"), true).toBool();
    return value;
}

void AppSettings::setProvider(const ProviderSettings& provider) {
    m_settings.setValue(QStringLiteral("provider/name"), provider.displayName.trimmed());
    m_settings.setValue(QStringLiteral("provider/chatEndpoint"),
                        provider.chatEndpoint.toString(QUrl::FullyEncoded));
    m_settings.setValue(QStringLiteral("provider/modelsEndpoint"),
                        provider.modelsEndpoint.toString(QUrl::FullyEncoded));
    m_settings.setValue(QStringLiteral("provider/model"), provider.model.trimmed());
    m_settings.setValue(QStringLiteral("provider/openRouter"), provider.openRouter);
    m_settings.setValue(QStringLiteral("privacy/denyDataCollection"),
                        provider.denyDataCollection);
    m_settings.setValue(QStringLiteral("privacy/zeroDataRetention"),
                        provider.zeroDataRetention);
    m_settings.setValue(QStringLiteral("provider/preferThroughput"), provider.preferThroughput);
    sync();
}

QString AppSettings::sourceLanguage() const {
    return m_settings.value(QStringLiteral("translation/source"), QStringLiteral("auto")).toString();
}

QString AppSettings::targetLanguage() const {
    return m_settings.value(QStringLiteral("translation/target"), QStringLiteral("en")).toString();
}

void AppSettings::setLanguagePair(const QString& sourceCode, const QString& targetCode) {
    m_settings.setValue(QStringLiteral("translation/source"), sourceCode);
    m_settings.setValue(QStringLiteral("translation/target"), targetCode);
    sync();
}

TranslationStyle AppSettings::translationStyle() const {
    const int stored = m_settings.value(QStringLiteral("translation/style"), 0).toInt();
    const int bounded = std::clamp(stored, 0, static_cast<int>(TranslationStyle::Casual));
    return static_cast<TranslationStyle>(bounded);
}

void AppSettings::setTranslationStyle(TranslationStyle style) {
    m_settings.setValue(QStringLiteral("translation/style"), static_cast<int>(style));
    sync();
}

bool AppSettings::preserveFormatting() const {
    return m_settings.value(QStringLiteral("translation/preserveFormatting"), true).toBool();
}

void AppSettings::setPreserveFormatting(bool enabled) {
    m_settings.setValue(QStringLiteral("translation/preserveFormatting"), enabled);
    sync();
}

QString AppSettings::customInstruction() const {
    return m_settings.value(QStringLiteral("translation/customInstruction")).toString();
}

void AppSettings::setCustomInstruction(const QString& instruction) {
    m_settings.setValue(QStringLiteral("translation/customInstruction"),
                        instruction.left(2000).trimmed());
    sync();
}

bool AppSettings::historyEnabled() const {
    return m_settings.value(QStringLiteral("privacy/historyEnabled"), false).toBool();
}

void AppSettings::setHistoryEnabled(bool enabled) {
    m_settings.setValue(QStringLiteral("privacy/historyEnabled"), enabled);
    sync();
}

int AppSettings::historyLimit() const {
    return std::clamp(m_settings.value(QStringLiteral("privacy/historyLimit"), 100).toInt(), 10,
                      1000);
}

void AppSettings::setHistoryLimit(int limit) {
    m_settings.setValue(QStringLiteral("privacy/historyLimit"), std::clamp(limit, 10, 1000));
    sync();
}

int AppSettings::historyRetentionDays() const {
    return std::clamp(m_settings.value(QStringLiteral("privacy/historyRetentionDays"), 30).toInt(),
                      1, 365);
}

void AppSettings::setHistoryRetentionDays(int days) {
    m_settings.setValue(QStringLiteral("privacy/historyRetentionDays"), std::clamp(days, 1, 365));
    sync();
}

bool AppSettings::startInTray() const {
    return m_settings.value(QStringLiteral("general/startInTray"), true).toBool();
}

void AppSettings::setStartInTray(bool enabled) {
    m_settings.setValue(QStringLiteral("general/startInTray"), enabled);
    sync();
}

bool AppSettings::closeToTray() const {
    return m_settings.value(QStringLiteral("general/closeToTray"), true).toBool();
}

void AppSettings::setCloseToTray(bool enabled) {
    m_settings.setValue(QStringLiteral("general/closeToTray"), enabled);
    sync();
}

QString AppSettings::interfaceLanguage() const {
    const QString stored =
        m_settings
            .value(QStringLiteral("general/interfaceLanguage"),
                   InterfaceLanguageManager::systemDefault())
            .toString();
    return InterfaceLanguageManager::normalize(stored);
}

void AppSettings::setInterfaceLanguage(const QString& languageCode) {
    m_settings.setValue(QStringLiteral("general/interfaceLanguage"),
                        InterfaceLanguageManager::normalize(languageCode));
    sync();
}

bool AppSettings::rememberApiKey() const {
    return m_settings.value(QStringLiteral("privacy/rememberApiKey"), true).toBool();
}

void AppSettings::setRememberApiKey(bool enabled) {
    m_settings.setValue(QStringLiteral("privacy/rememberApiKey"), enabled);
    sync();
}

int AppSettings::maximumInputCharacters() const {
    return std::clamp(
        m_settings.value(QStringLiteral("translation/maximumInputCharacters"), 50000).toInt(),
        1000, 200000);
}

void AppSettings::setMaximumInputCharacters(int value) {
    m_settings.setValue(QStringLiteral("translation/maximumInputCharacters"),
                        std::clamp(value, 1000, 200000));
    sync();
}

QString AppSettings::photoOcrLanguage() const {
    return m_settings
        .value(QStringLiteral("photoOcr/language"), QStringLiteral("match-source"))
        .toString();
}

void AppSettings::setPhotoOcrLanguage(const QString& language) {
    const QString normalized = language.trimmed();
    m_settings.setValue(QStringLiteral("photoOcr/language"),
                        normalized.isEmpty() ? QStringLiteral("match-source") : normalized);
    sync();
}

int AppSettings::photoOcrLayout() const {
    return std::clamp(m_settings.value(QStringLiteral("photoOcr/layout"), 0).toInt(), 0, 2);
}

void AppSettings::setPhotoOcrLayout(int layout) {
    m_settings.setValue(QStringLiteral("photoOcr/layout"), std::clamp(layout, 0, 2));
    sync();
}

QString AppSettings::storagePath() const {
    return m_settings.fileName();
}

QString AppSettings::storageError() const {
    return m_storageError;
}

bool AppSettings::storageHealthy() const {
    return m_storageError.isEmpty();
}

void AppSettings::resetProvider() {
    const QStringList keys = {QStringLiteral("provider/name"),
                              QStringLiteral("provider/chatEndpoint"),
                              QStringLiteral("provider/modelsEndpoint"),
                              QStringLiteral("provider/model"),
                              QStringLiteral("provider/openRouter"),
                              QStringLiteral("provider/preferThroughput"),
                              QStringLiteral("privacy/denyDataCollection"),
                              QStringLiteral("privacy/zeroDataRetention")};
    for (const QString& key : keys) {
        m_settings.remove(key);
    }
    sync();
}

void AppSettings::sync() {
    const QFileInfo beforeSync(m_settings.fileName());
    if (beforeSync.isSymLink()) {
        setStorageError(tr("The settings file is a symbolic link and will not be written: %1")
                            .arg(m_settings.fileName()));
        emit changed();
        return;
    }

    m_settings.sync();

    QString error;
    switch (m_settings.status()) {
    case QSettings::NoError:
        break;
    case QSettings::AccessError:
        error = tr("Verbuno cannot write its local settings file: %1")
                    .arg(m_settings.fileName());
        break;
    case QSettings::FormatError:
        error = tr("The local settings file is damaged or has an invalid format: %1")
                    .arg(m_settings.fileName());
        break;
    }

    const QFileInfo afterSync(m_settings.fileName());
    if (error.isEmpty() && afterSync.exists() &&
        !QFile::setPermissions(m_settings.fileName(),
                               QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
        error = tr("Verbuno could not restrict the settings file to the current user: %1")
                    .arg(m_settings.fileName());
    }

    setStorageError(error);
    emit changed();
}

void AppSettings::setStorageError(const QString& error) {
    if (m_storageError == error) {
        return;
    }
    m_storageError = error;
    emit storageStatusChanged(m_storageError);
}

} // namespace verbuno
