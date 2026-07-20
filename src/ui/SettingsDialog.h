#pragma once

#include "core/TranslationTypes.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;

namespace verbuno {

class AppSettings;
class TranslationController;

class SettingsDialog final : public QDialog {
    Q_OBJECT

public:
    SettingsDialog(AppSettings* settings,
                   TranslationController* controller,
                   QWidget* parent = nullptr);

signals:
    void interfaceLanguageChanged(const QString& languageCode);

private:
    QWidget* createProviderPage();
    QWidget* createTranslationPage();
    QWidget* createPrivacyPage();
    QWidget* createGeneralPage();
    void loadSettings();
    [[nodiscard]] ProviderSettings providerFromUi() const;
    [[nodiscard]] bool saveProviderDraft();
    void saveAndClose();
    void finishSaveAndClose();
    void setKeySaveBusy(bool busy);
    void updateStorageStatus();
    void updateProviderControls();
    void populateModels(const QVector<ModelInfo>& models);

    AppSettings* m_settings;
    TranslationController* m_controller;

    QComboBox* m_providerType = nullptr;
    QLineEdit* m_providerName = nullptr;
    QLineEdit* m_endpoint = nullptr;
    QComboBox* m_model = nullptr;
    QLineEdit* m_apiKey = nullptr;
    QCheckBox* m_rememberKey = nullptr;
    QPushButton* m_saveKeyButton = nullptr;
    QCheckBox* m_denyCollection = nullptr;
    QCheckBox* m_zeroRetention = nullptr;
    QCheckBox* m_preferThroughput = nullptr;
    QLabel* m_providerStatus = nullptr;

    QComboBox* m_style = nullptr;
    QCheckBox* m_preserveFormatting = nullptr;
    QPlainTextEdit* m_customInstruction = nullptr;
    QSpinBox* m_inputLimit = nullptr;

    QCheckBox* m_historyEnabled = nullptr;
    QSpinBox* m_historyLimit = nullptr;
    QSpinBox* m_retentionDays = nullptr;

    QCheckBox* m_startInTray = nullptr;
    QCheckBox* m_closeToTray = nullptr;
    QComboBox* m_interfaceLanguage = nullptr;
    QLabel* m_storageStatus = nullptr;
    QDialogButtonBox* m_buttons = nullptr;

    bool m_keySaveInProgress = false;
    bool m_closeAfterKeySave = false;
};

} // namespace verbuno
