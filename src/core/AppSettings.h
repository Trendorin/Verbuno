#pragma once

#include "core/TranslationTypes.h"

#include <QObject>
#include <QSettings>

namespace verbuno {

class AppSettings final : public QObject {
    Q_OBJECT

public:
    explicit AppSettings(QObject* parent = nullptr);

    [[nodiscard]] ProviderSettings provider() const;
    void setProvider(const ProviderSettings& provider);

    [[nodiscard]] QString sourceLanguage() const;
    [[nodiscard]] QString targetLanguage() const;
    void setLanguagePair(const QString& sourceCode, const QString& targetCode);

    [[nodiscard]] TranslationStyle translationStyle() const;
    void setTranslationStyle(TranslationStyle style);

    [[nodiscard]] bool preserveFormatting() const;
    void setPreserveFormatting(bool enabled);

    [[nodiscard]] QString customInstruction() const;
    void setCustomInstruction(const QString& instruction);

    [[nodiscard]] bool historyEnabled() const;
    void setHistoryEnabled(bool enabled);

    [[nodiscard]] int historyLimit() const;
    void setHistoryLimit(int limit);

    [[nodiscard]] int historyRetentionDays() const;
    void setHistoryRetentionDays(int days);

    [[nodiscard]] bool startInTray() const;
    void setStartInTray(bool enabled);

    [[nodiscard]] bool closeToTray() const;
    void setCloseToTray(bool enabled);

    [[nodiscard]] QString interfaceLanguage() const;
    void setInterfaceLanguage(const QString& languageCode);

    [[nodiscard]] bool rememberApiKey() const;
    void setRememberApiKey(bool enabled);

    [[nodiscard]] int maximumInputCharacters() const;
    void setMaximumInputCharacters(int value);

    [[nodiscard]] QString photoOcrLanguage() const;
    void setPhotoOcrLanguage(const QString& language);

    [[nodiscard]] int photoOcrLayout() const;
    void setPhotoOcrLayout(int layout);

    [[nodiscard]] QString storagePath() const;
    [[nodiscard]] QString storageError() const;
    [[nodiscard]] bool storageHealthy() const;

    void resetProvider();

signals:
    void changed();
    void storageStatusChanged(const QString& error);

private:
    void sync();
    void setStorageError(const QString& error);

    QSettings m_settings;
    QString m_storageError;
};

} // namespace verbuno
