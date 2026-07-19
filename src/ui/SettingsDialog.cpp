#include "ui/SettingsDialog.h"

#include "core/AppSettings.h"
#include "core/EndpointValidator.h"
#include "core/TranslationController.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

namespace translunix {

namespace {
const QUrl kOpenRouterChat(QStringLiteral("https://openrouter.ai/api/v1/chat/completions"));
const QUrl kOpenRouterModels(QStringLiteral("https://openrouter.ai/api/v1/models"));
}

SettingsDialog::SettingsDialog(AppSettings* settings,
                               TranslationController* controller,
                               QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_controller(controller) {
    setWindowTitle(tr("TranslUnix Settings"));
    setMinimumSize(720, 610);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(createProviderPage(), tr("Provider"));
    tabs->addTab(createTranslationPage(), tr("Translation"));
    tabs->addTab(createPrivacyPage(), tr("Privacy"));
    tabs->addTab(createGeneralPage(), tr("General"));
    root->addWidget(tabs, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel |
                                              QDialogButtonBox::RestoreDefaults,
                                          this);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::saveAndClose);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this,
            [this] {
                m_settings->resetProvider();
                loadSettings();
                m_providerStatus->setText(tr("OpenRouter defaults restored."));
            });
    root->addWidget(buttons);

    connect(m_controller, &TranslationController::modelsLoaded, this,
            &SettingsDialog::populateModels);
    connect(m_controller, &TranslationController::modelsFailed, this,
            [this](const QString& error) { m_providerStatus->setText(error); });
    connect(m_controller, &TranslationController::apiKeyStored, this,
            [this](bool persistent, const QString& error) {
                if (!error.isEmpty()) {
                    m_providerStatus->setText(error);
                    return;
                }
                m_apiKey->clear();
                m_providerStatus->setText(
                    persistent ? tr("API key saved in the system keychain.")
                               : tr("API key kept in memory for this session."));
            });
    connect(m_controller, &TranslationController::apiKeyDeleted, this,
            [this](const QString& error) {
                m_providerStatus->setText(error.isEmpty() ? tr("API key removed.") : error);
            });

    loadSettings();
}

QWidget* SettingsDialog::createProviderPage() {
    auto* page = new QWidget(this);
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(14);

    auto* providerBox = new QGroupBox(tr("API provider"), page);
    auto* form = new QFormLayout(providerBox);
    m_providerType = new QComboBox(providerBox);
    m_providerType->addItem(QStringLiteral("OpenRouter"), true);
    m_providerType->addItem(tr("OpenAI-compatible endpoint"), false);
    m_providerName = new QLineEdit(providerBox);
    m_endpoint = new QLineEdit(providerBox);
    m_endpoint->setPlaceholderText(QStringLiteral("https://provider.example/v1/chat/completions"));
    m_model = new QComboBox(providerBox);
    m_model->setEditable(true);
    m_model->setInsertPolicy(QComboBox::NoInsert);
    m_model->addItem(QStringLiteral("OpenRouter Free Models Router"),
                     QStringLiteral("openrouter/free"));
    form->addRow(tr("Type"), m_providerType);
    form->addRow(tr("Name"), m_providerName);
    form->addRow(tr("Chat endpoint"), m_endpoint);
    form->addRow(tr("Model ID"), m_model);
    root->addWidget(providerBox);

    auto* modelRow = new QHBoxLayout;
    auto* refreshModels = new QPushButton(tr("Refresh free OpenRouter models"), page);
    modelRow->addWidget(refreshModels);
    modelRow->addStretch();
    root->addLayout(modelRow);

    auto* keyBox = new QGroupBox(tr("API key"), page);
    auto* keyLayout = new QVBoxLayout(keyBox);
    m_apiKey = new QLineEdit(keyBox);
    m_apiKey->setEchoMode(QLineEdit::Password);
    m_apiKey->setPlaceholderText(tr("Paste a new key; stored keys are never shown"));
    m_apiKey->setClearButtonEnabled(true);
    m_rememberKey = new QCheckBox(tr("Remember in KWallet / Secret Service"), keyBox);
    auto* keyButtons = new QHBoxLayout;
    auto* saveKey = new QPushButton(tr("Save key"), keyBox);
    auto* clearKey = new QPushButton(tr("Remove key"), keyBox);
    keyButtons->addWidget(saveKey);
    keyButtons->addWidget(clearKey);
    keyButtons->addStretch();
    keyLayout->addWidget(m_apiKey);
    keyLayout->addWidget(m_rememberKey);
    keyLayout->addLayout(keyButtons);
    root->addWidget(keyBox);

    auto* routingBox = new QGroupBox(tr("OpenRouter routing"), page);
    auto* routingLayout = new QVBoxLayout(routingBox);
    m_denyCollection =
        new QCheckBox(tr("Exclude providers that collect prompt data (recommended)"), routingBox);
    m_zeroRetention =
        new QCheckBox(tr("Require Zero Data Retention endpoints (strict; fewer models)"), routingBox);
    m_preferThroughput = new QCheckBox(tr("Prefer faster providers"), routingBox);
    routingLayout->addWidget(m_denyCollection);
    routingLayout->addWidget(m_zeroRetention);
    routingLayout->addWidget(m_preferThroughput);
    root->addWidget(routingBox);

    auto* notice = new QLabel(
        tr("TranslUnix sends text only to this endpoint. OpenRouter is a proxy: the selected "
           "upstream provider may have its own retention or training policy. Free endpoints can "
           "be rate-limited or unavailable. Review OpenRouter privacy settings before sending "
           "sensitive content."),
        page);
    notice->setWordWrap(true);
    root->addWidget(notice);

    m_providerStatus = new QLabel(page);
    m_providerStatus->setWordWrap(true);
    m_providerStatus->setTextInteractionFlags(Qt::TextSelectableByMouse);
    root->addWidget(m_providerStatus);
    root->addStretch();

    connect(m_providerType, &QComboBox::currentIndexChanged, this,
            &SettingsDialog::updateProviderControls);
    connect(refreshModels, &QPushButton::clicked, this, [this] {
        if (!saveProviderDraft()) {
            return;
        }
        m_providerStatus->setText(tr("Loading the current free-model catalog…"));
        m_controller->refreshFreeModels();
    });
    connect(saveKey, &QPushButton::clicked, this, [this] {
        if (!saveProviderDraft()) {
            return;
        }
        if (m_apiKey->text().trimmed().isEmpty()) {
            m_providerStatus->setText(tr("Paste a new API key first."));
            return;
        }
        m_settings->setRememberApiKey(m_rememberKey->isChecked());
        m_controller->saveApiKey(m_apiKey->text(), m_rememberKey->isChecked());
    });
    connect(clearKey, &QPushButton::clicked, this, [this] {
        if (!saveProviderDraft()) {
            return;
        }
        m_controller->clearApiKey();
    });
    return page;
}

QWidget* SettingsDialog::createTranslationPage() {
    auto* page = new QWidget(this);
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(14);

    auto* form = new QFormLayout;
    m_style = new QComboBox(page);
    m_style->addItem(tr("Natural"), static_cast<int>(TranslationStyle::Natural));
    m_style->addItem(tr("Formal"), static_cast<int>(TranslationStyle::Formal));
    m_style->addItem(tr("Literal"), static_cast<int>(TranslationStyle::Literal));
    m_style->addItem(tr("Technical"), static_cast<int>(TranslationStyle::Technical));
    m_style->addItem(tr("Casual"), static_cast<int>(TranslationStyle::Casual));
    m_inputLimit = new QSpinBox(page);
    m_inputLimit->setRange(1000, 200000);
    m_inputLimit->setSingleStep(1000);
    m_inputLimit->setSuffix(tr(" characters"));
    form->addRow(tr("Default style"), m_style);
    form->addRow(tr("Maximum input"), m_inputLimit);
    root->addLayout(form);

    m_preserveFormatting =
        new QCheckBox(tr("Preserve paragraphs, Markdown, lists, and line breaks"), page);
    root->addWidget(m_preserveFormatting);

    root->addWidget(new QLabel(tr("Custom translation preference"), page));
    m_customInstruction = new QPlainTextEdit(page);
    m_customInstruction->setPlaceholderText(
        tr("For example: keep product names in English and use concise technical terminology"));
    m_customInstruction->setMaximumHeight(150);
    root->addWidget(m_customInstruction);

    auto* note = new QLabel(
        tr("The built-in prompt already preserves placeholders, code, URLs, tags, names, numbers, "
           "and treats instructions inside translated text as untrusted content."),
        page);
    note->setWordWrap(true);
    root->addWidget(note);
    root->addStretch();
    return page;
}

QWidget* SettingsDialog::createPrivacyPage() {
    auto* page = new QWidget(this);
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(14);

    m_historyEnabled = new QCheckBox(tr("Keep a local translation history"), page);
    root->addWidget(m_historyEnabled);

    auto* form = new QFormLayout;
    m_historyLimit = new QSpinBox(page);
    m_historyLimit->setRange(10, 1000);
    m_retentionDays = new QSpinBox(page);
    m_retentionDays->setRange(1, 365);
    m_retentionDays->setSuffix(tr(" days"));
    form->addRow(tr("Maximum records"), m_historyLimit);
    form->addRow(tr("Delete records after"), m_retentionDays);
    root->addLayout(form);

    auto* clearHistory = new QPushButton(tr("Delete local history now"), page);
    root->addWidget(clearHistory, 0, Qt::AlignLeft);

    auto* explanation = new QLabel(
        tr("History is off by default. When enabled, it is stored only in your user data "
           "directory using an owner-only file and atomic writes. API keys are never written to "
           "history or normal settings."),
        page);
    explanation->setWordWrap(true);
    root->addWidget(explanation);
    root->addStretch();

    connect(clearHistory, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, tr("Delete history"),
                                  tr("Delete every locally stored translation?")) ==
            QMessageBox::Yes) {
            m_controller->clearHistory();
        }
    });
    return page;
}

QWidget* SettingsDialog::createGeneralPage() {
    auto* page = new QWidget(this);
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(14);

    m_startInTray = new QCheckBox(tr("Start in the system tray"), page);
    m_closeToTray = new QCheckBox(tr("Closing the main window keeps TranslUnix in the tray"), page);
    root->addWidget(m_startInTray);
    root->addWidget(m_closeToTray);

    auto* shortcut = new QLabel(
        tr("Desktop shortcut command: <code>translunix --toggle</code><br>"
           "Bind this command in KDE or GNOME keyboard settings for a compositor-safe global "
           "shortcut."),
        page);
    shortcut->setWordWrap(true);
    shortcut->setTextInteractionFlags(Qt::TextSelectableByMouse);
    root->addWidget(shortcut);

    auto* desktopNote = new QLabel(
        tr("KDE Plasma has native tray support. GNOME may require an AppIndicator extension; when "
           "no tray is available, TranslUnix opens the main window instead."),
        page);
    desktopNote->setWordWrap(true);
    root->addWidget(desktopNote);
    root->addStretch();
    return page;
}

void SettingsDialog::loadSettings() {
    const ProviderSettings provider = m_settings->provider();
    m_providerType->setCurrentIndex(provider.openRouter ? 0 : 1);
    m_providerName->setText(provider.displayName);
    m_endpoint->setText(provider.chatEndpoint.toString(QUrl::FullyEncoded));
    int modelIndex = m_model->findData(provider.model);
    if (modelIndex < 0) {
        m_model->addItem(provider.model, provider.model);
        modelIndex = m_model->count() - 1;
    }
    m_model->setCurrentIndex(modelIndex);
    m_model->setEditText(provider.model);
    m_rememberKey->setChecked(m_settings->rememberApiKey());
    m_denyCollection->setChecked(provider.denyDataCollection);
    m_zeroRetention->setChecked(provider.zeroDataRetention);
    m_preferThroughput->setChecked(provider.preferThroughput);

    const int styleIndex = m_style->findData(static_cast<int>(m_settings->translationStyle()));
    if (styleIndex >= 0) {
        m_style->setCurrentIndex(styleIndex);
    }
    m_preserveFormatting->setChecked(m_settings->preserveFormatting());
    m_customInstruction->setPlainText(m_settings->customInstruction());
    m_inputLimit->setValue(m_settings->maximumInputCharacters());

    m_historyEnabled->setChecked(m_settings->historyEnabled());
    m_historyLimit->setValue(m_settings->historyLimit());
    m_retentionDays->setValue(m_settings->historyRetentionDays());
    m_startInTray->setChecked(m_settings->startInTray());
    m_closeToTray->setChecked(m_settings->closeToTray());
    updateProviderControls();
}

ProviderSettings SettingsDialog::providerFromUi() const {
    ProviderSettings provider;
    provider.openRouter = m_providerType->currentData().toBool();
    provider.displayName = m_providerName->text().trimmed();
    if (provider.displayName.isEmpty()) {
        provider.displayName = provider.openRouter ? QStringLiteral("OpenRouter")
                                                   : QStringLiteral("Custom provider");
    }
    provider.chatEndpoint = QUrl::fromUserInput(m_endpoint->text().trimmed());
    provider.modelsEndpoint = provider.openRouter ? kOpenRouterModels : QUrl();
    const QString modelText = m_model->currentText().trimmed();
    const int listedModel = m_model->findText(modelText, Qt::MatchExactly);
    provider.model = listedModel >= 0 && !m_model->itemData(listedModel).toString().isEmpty()
                         ? m_model->itemData(listedModel).toString()
                         : modelText;
    provider.denyDataCollection = m_denyCollection->isChecked();
    provider.zeroDataRetention = m_zeroRetention->isChecked();
    provider.preferThroughput = m_preferThroughput->isChecked();
    return provider;
}

bool SettingsDialog::saveProviderDraft() {
    const ProviderSettings provider = providerFromUi();
    const EndpointValidation validation = EndpointValidator::validate(provider.chatEndpoint);
    if (!validation.valid) {
        m_providerStatus->setText(validation.error);
        return false;
    }
    if (provider.model.isEmpty()) {
        m_providerStatus->setText(tr("Enter a model identifier."));
        return false;
    }
    m_settings->setProvider(provider);
    return true;
}

void SettingsDialog::saveAndClose() {
    if (!saveProviderDraft()) {
        return;
    }
    m_settings->setRememberApiKey(m_rememberKey->isChecked());
    m_settings->setTranslationStyle(
        static_cast<TranslationStyle>(m_style->currentData().toInt()));
    m_settings->setPreserveFormatting(m_preserveFormatting->isChecked());
    m_settings->setCustomInstruction(m_customInstruction->toPlainText());
    m_settings->setMaximumInputCharacters(m_inputLimit->value());
    m_settings->setHistoryEnabled(m_historyEnabled->isChecked());
    m_settings->setHistoryLimit(m_historyLimit->value());
    m_settings->setHistoryRetentionDays(m_retentionDays->value());
    m_settings->setStartInTray(m_startInTray->isChecked());
    m_settings->setCloseToTray(m_closeToTray->isChecked());
    if (!m_apiKey->text().trimmed().isEmpty()) {
        m_controller->saveApiKey(m_apiKey->text(), m_rememberKey->isChecked());
    }
    accept();
}

void SettingsDialog::updateProviderControls() {
    const bool openRouter = m_providerType->currentData().toBool();
    m_denyCollection->setEnabled(openRouter);
    m_zeroRetention->setEnabled(openRouter);
    m_preferThroughput->setEnabled(openRouter);
    if (openRouter && (m_endpoint->text().isEmpty() ||
                       !EndpointValidator::isOpenRouter(QUrl(m_endpoint->text())))) {
        m_providerName->setText(QStringLiteral("OpenRouter"));
        m_endpoint->setText(kOpenRouterChat.toString());
    }
}

void SettingsDialog::populateModels(const QVector<ModelInfo>& models) {
    const QString previous = m_model->currentText();
    m_model->clear();
    for (const ModelInfo& model : models) {
        QString label = model.name;
        if (model.contextLength > 0) {
            label += tr(" · %1k context").arg(model.contextLength / 1000);
        }
        m_model->addItem(label, model.id);
    }
    int index = m_model->findData(previous);
    if (index < 0) {
        index = m_model->findText(previous);
    }
    if (index >= 0) {
        m_model->setCurrentIndex(index);
    }
    m_providerStatus->setText(tr("Loaded %1 currently free models.").arg(models.size()));
}

} // namespace translunix
