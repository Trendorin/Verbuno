#include "ui/TranslationPanel.h"

#include "core/AppSettings.h"
#include "core/LanguageCatalog.h"
#include "core/TranslationController.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShortcut>
#include <QSizePolicy>
#include <QTextCursor>
#include <QToolButton>
#include <QVBoxLayout>

namespace verbuno {

TranslationPanel::TranslationPanel(TranslationController* controller,
                                   AppSettings* settings,
                                   QWidget* parent)
    : QWidget(parent)
    , m_controller(controller)
    , m_settings(settings) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    m_providerSummary = new QLabel(this);
    m_providerSummary->setTextFormat(Qt::PlainText);
    m_providerSummary->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_providerSummary->setWordWrap(true);
    m_providerSummary->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    QFont providerFont = m_providerSummary->font();
    providerFont.setBold(true);
    m_providerSummary->setFont(providerFont);
    root->addWidget(m_providerSummary);

    auto* languageRow = new QHBoxLayout;
    languageRow->setSpacing(8);
    m_sourceCombo = new QComboBox(this);
    m_sourceCombo->setAccessibleName(tr("Source language"));
    m_sourceCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_sourceCombo->setMinimumContentsLength(12);
    m_targetCombo = new QComboBox(this);
    m_targetCombo->setAccessibleName(tr("Target language"));
    m_targetCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_targetCombo->setMinimumContentsLength(12);
    m_swapButton = new QToolButton(this);
    m_swapButton->setText(QStringLiteral("⇄"));
    m_swapButton->setToolTip(tr("Swap languages"));
    m_swapButton->setAccessibleName(tr("Swap languages"));
    languageRow->addWidget(m_sourceCombo, 1);
    languageRow->addWidget(m_swapButton);
    languageRow->addWidget(m_targetCombo, 1);
    root->addLayout(languageRow);

    auto* optionRow = new QHBoxLayout;
    auto* styleLabel = new QLabel(tr("Style"), this);
    m_styleCombo = new QComboBox(this);
    m_styleCombo->addItem(tr("Natural"), static_cast<int>(TranslationStyle::Natural));
    m_styleCombo->addItem(tr("Formal"), static_cast<int>(TranslationStyle::Formal));
    m_styleCombo->addItem(tr("Literal"), static_cast<int>(TranslationStyle::Literal));
    m_styleCombo->addItem(tr("Technical"), static_cast<int>(TranslationStyle::Technical));
    m_styleCombo->addItem(tr("Casual"), static_cast<int>(TranslationStyle::Casual));
    const int styleIndex = m_styleCombo->findData(static_cast<int>(m_settings->translationStyle()));
    if (styleIndex >= 0) {
        m_styleCombo->setCurrentIndex(styleIndex);
    }
    optionRow->addWidget(styleLabel);
    optionRow->addWidget(m_styleCombo);
    optionRow->addStretch();
    root->addLayout(optionRow);

    auto* inputHeader = new QHBoxLayout;
    inputHeader->addWidget(new QLabel(tr("Text"), this));
    inputHeader->addStretch();
    m_counter = new QLabel(this);
    inputHeader->addWidget(m_counter);
    root->addLayout(inputHeader);

    m_input = new QPlainTextEdit(this);
    m_input->setPlaceholderText(tr("Enter or paste text to translate"));
    m_input->setMinimumHeight(190);
    root->addWidget(m_input, 1);

    auto* actionRow = new QHBoxLayout;
    auto* clearButton = new QPushButton(tr("Clear"), this);
    m_translateButton = new QPushButton(tr("Translate"), this);
    m_translateButton->setDefault(true);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_cancelButton->setVisible(false);
    actionRow->addWidget(clearButton);
    actionRow->addStretch();
    actionRow->addWidget(m_cancelButton);
    actionRow->addWidget(m_translateButton);
    root->addLayout(actionRow);

    auto* outputHeader = new QHBoxLayout;
    outputHeader->addWidget(new QLabel(tr("Translation"), this));
    outputHeader->addStretch();
    auto* copyButton = new QPushButton(tr("Copy"), this);
    outputHeader->addWidget(copyButton);
    root->addLayout(outputHeader);

    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setPlaceholderText(tr("The streamed translation will appear here"));
    m_output->setMinimumHeight(190);
    root->addWidget(m_output, 1);

    m_status = new QLabel(this);
    m_status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_status->setWordWrap(true);
    root->addWidget(m_status);

    m_privacy = new QLabel(this);
    m_privacy->setWordWrap(true);
    root->addWidget(m_privacy);

    populateLanguages();
    updateCounter();
    updateControls();
    updateProviderDisplay();

    connect(m_input, &QPlainTextEdit::textChanged, this, [this] {
        updateCounter();
        updateControls();
    });
    connect(m_sourceCombo, &QComboBox::currentIndexChanged, this, [this] {
        m_settings->setLanguagePair(currentSourceCode(), currentTargetCode());
        updateControls();
    });
    connect(m_targetCombo, &QComboBox::currentIndexChanged, this, [this] {
        m_settings->setLanguagePair(currentSourceCode(), currentTargetCode());
        updateControls();
    });
    if (m_styleCombo) {
        connect(m_styleCombo, &QComboBox::currentIndexChanged, this, [this] {
            m_settings->setTranslationStyle(
                static_cast<TranslationStyle>(m_styleCombo->currentData().toInt()));
        });
    }
    connect(m_swapButton, &QToolButton::clicked, this, &TranslationPanel::swapLanguages);
    connect(m_translateButton, &QPushButton::clicked, this,
            &TranslationPanel::requestTranslation);
    connect(m_cancelButton, &QPushButton::clicked, m_controller,
            &TranslationController::cancel);
    connect(clearButton, &QPushButton::clicked, this, [this] {
        m_input->clear();
        m_output->clear();
        m_status->clear();
        focusInput();
    });
    connect(copyButton, &QPushButton::clicked, this, [this] {
        if (!m_output->toPlainText().isEmpty()) {
            QApplication::clipboard()->setText(m_output->toPlainText());
            m_status->setText(tr("Translation copied to the clipboard."));
        }
    });

    auto* translateShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this);
    connect(translateShortcut, &QShortcut::activated, this,
            &TranslationPanel::requestTranslation);

    connect(m_controller, &TranslationController::requestStarted, this,
            &TranslationPanel::beginRequest);
    connect(m_controller, &TranslationController::translationChunk, this,
            &TranslationPanel::appendChunk);
    connect(m_controller, &TranslationController::inferenceRouteChanged, this,
            [this] { updateProviderDisplay(); });
    connect(m_controller, &TranslationController::translationFinished, this,
            &TranslationPanel::finishRequest);
    connect(m_controller, &TranslationController::requestFailed, this,
            &TranslationPanel::showError);
    connect(m_settings, &AppSettings::changed, this, [this] {
        updateCounter();
        updateControls();
        updateProviderDisplay();
    });
    if (m_controller->isBusy()) {
        m_status->setText(tr("Connecting to the selected model…"));
        setBusy(true);
    }
}

void TranslationPanel::focusInput() {
    m_input->setFocus(Qt::OtherFocusReason);
}

QString TranslationPanel::inputText() const {
    return m_input->toPlainText();
}

void TranslationPanel::setInputText(const QString& text) {
    m_input->setPlainText(text);
}

QString TranslationPanel::outputText() const {
    return m_output->toPlainText();
}

void TranslationPanel::setOutputText(const QString& text) {
    m_output->setPlainText(text);
}

void TranslationPanel::populateLanguages() {
    m_sourceCombo->blockSignals(true);
    m_targetCombo->blockSignals(true);
    m_sourceCombo->clear();
    m_targetCombo->clear();
    m_sourceCombo->addItem(tr("Detect automatically"), QStringLiteral("auto"));
    for (const Language& language : LanguageCatalog::all()) {
        const QString label = QStringLiteral("%1 · %2").arg(language.name, language.code);
        m_sourceCombo->addItem(label, language.code);
        m_targetCombo->addItem(label, language.code);
    }
    selectCode(m_sourceCombo, m_settings->sourceLanguage());
    selectCode(m_targetCombo, m_settings->targetLanguage());
    m_sourceCombo->blockSignals(false);
    m_targetCombo->blockSignals(false);
}

void TranslationPanel::updateCounter() {
    const qsizetype count = m_input->toPlainText().size();
    m_counter->setText(QStringLiteral("%1 / %2")
                           .arg(static_cast<qlonglong>(count))
                           .arg(m_settings->maximumInputCharacters()));
}

void TranslationPanel::updateControls() {
    const bool hasInput = !m_input->toPlainText().trimmed().isEmpty();
    const bool withinLimit =
        m_input->toPlainText().size() <= m_settings->maximumInputCharacters();
    const bool pairValid = currentSourceCode() == QStringLiteral("auto") ||
                           currentSourceCode() != currentTargetCode();
    m_translateButton->setEnabled(!m_busy && hasInput && withinLimit && pairValid);
    m_swapButton->setEnabled(!m_busy);
    m_sourceCombo->setEnabled(!m_busy);
    m_targetCombo->setEnabled(!m_busy);
}

void TranslationPanel::updateProviderDisplay() {
    const ProviderSettings provider = m_settings->provider();
    const QString storedName = provider.displayName.trimmed();
    const bool defaultCustomName =
        !provider.openRouter &&
        (storedName.isEmpty() || storedName == QStringLiteral("Custom provider"));
    const QString providerName = provider.openRouter
                                     ? QStringLiteral("OpenRouter")
                                     : (defaultCustomName ? tr("Custom provider") : storedName);
    const bool routeMatches = m_controller->inferenceRouteMatchesCurrentProvider();
    const InferenceRoute route = routeMatches ? m_controller->inferenceRoute() : InferenceRoute{};
    const QString actualProvider =
        provider.openRouter && !route.provider.isEmpty()
            ? tr("%1 via OpenRouter").arg(route.provider)
            : providerName;
    const QString actualModel =
        !route.model.isEmpty()
            ? route.model
            : (routeMatches && m_controller->isBusy() ? tr("resolving…")
                                                       : tr("not reported yet"));
    m_providerSummary->setText(
        tr("Provider: %1 · Actual model: %2").arg(actualProvider, actualModel));
    m_providerSummary->setToolTip(
        tr("Requested model or route: %1\nEndpoint: %2")
            .arg(provider.model.trimmed(),
                 provider.chatEndpoint.toString(QUrl::FullyEncoded)));

    if (provider.openRouter && provider.zeroDataRetention) {
        m_privacy->setText(tr("Strict route: Zero Data Retention endpoints only"));
    } else if (provider.openRouter && provider.denyDataCollection) {
        m_privacy->setText(tr("Private route: providers that collect prompts are excluded"));
    } else {
        m_privacy->setText(tr("Provider policy applies; review it before sending sensitive text"));
    }
}

void TranslationPanel::requestTranslation() {
    if (!m_translateButton->isEnabled()) {
        return;
    }
    m_controller->translate(m_input->toPlainText(), currentSourceCode(), currentTargetCode());
}

void TranslationPanel::swapLanguages() {
    const QString source = currentSourceCode();
    const QString target = currentTargetCode();
    if (source == QStringLiteral("auto")) {
        selectCode(m_sourceCombo, target);
        selectCode(m_targetCombo,
                   target == QStringLiteral("en") ? QStringLiteral("ru") : QStringLiteral("en"));
    } else {
        selectCode(m_sourceCombo, target);
        selectCode(m_targetCombo, source);
    }
}

void TranslationPanel::beginRequest() {
    m_output->clear();
    m_status->setText(tr("Connecting to the selected model…"));
    setBusy(true);
    updateProviderDisplay();
}

void TranslationPanel::appendChunk(const QString& chunk) {
    QTextCursor cursor = m_output->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(chunk);
    m_output->setTextCursor(cursor);
    m_output->ensureCursorVisible();
    m_status->setText(tr("Translating…"));
}

void TranslationPanel::finishRequest(const QString& result) {
    m_output->setPlainText(result);
    m_status->setText(tr("Translation complete."));
    setBusy(false);
    updateProviderDisplay();
}

void TranslationPanel::showError(const QString& message) {
    m_status->setText(message);
    setBusy(false);
    updateProviderDisplay();
}

void TranslationPanel::setBusy(bool busy) {
    m_busy = busy;
    m_cancelButton->setVisible(busy);
    m_translateButton->setVisible(!busy);
    m_input->setReadOnly(busy);
    updateControls();
}

QString TranslationPanel::currentSourceCode() const {
    return m_sourceCombo->currentData().toString();
}

QString TranslationPanel::currentTargetCode() const {
    return m_targetCombo->currentData().toString();
}

void TranslationPanel::selectCode(QComboBox* combo, const QString& code) {
    const int index = combo->findData(code);
    if (index >= 0) {
        combo->setCurrentIndex(index);
    }
}

} // namespace verbuno
