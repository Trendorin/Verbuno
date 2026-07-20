#include "ui/TranslationPanel.h"

#include "core/AppSettings.h"
#include "core/LanguageCatalog.h"
#include "core/PhotoOcrEngine.h"
#include "core/TranslationController.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QShortcut>
#include <QSizePolicy>
#include <QTextCursor>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace verbuno {

TranslationPanel::TranslationPanel(TranslationController* controller,
                                   AppSettings* settings,
                                   QWidget* parent)
    : QWidget(parent)
    , m_controller(controller)
    , m_settings(settings)
    , m_ocrEngine(new PhotoOcrEngine(this)) {
    setAcceptDrops(true);
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(8);

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

    m_photoGroup = new QGroupBox(tr("Text from photo"), this);
    auto* photoRoot = new QHBoxLayout(m_photoGroup);
    photoRoot->setContentsMargins(10, 8, 10, 8);
    photoRoot->setSpacing(10);

    m_photoPreview = new QLabel(m_photoGroup);
    m_photoPreview->setAlignment(Qt::AlignCenter);
    m_photoPreview->setFrameShape(QFrame::StyledPanel);
    m_photoPreview->setMinimumSize(128, 88);
    m_photoPreview->setMaximumSize(160, 108);
    m_photoPreview->setAccessibleName(tr("Selected photo preview"));
    m_photoPreview->setVisible(false);
    photoRoot->addWidget(m_photoPreview);

    auto* photoControls = new QVBoxLayout;
    photoControls->setSpacing(7);
    auto* photoActions = new QHBoxLayout;
    m_openPhotoButton = new QPushButton(tr("Open photo…"), m_photoGroup);
    m_pastePhotoButton = new QPushButton(tr("Paste photo"), m_photoGroup);
    photoActions->addWidget(m_openPhotoButton);
    photoActions->addWidget(m_pastePhotoButton);
    photoActions->addStretch();
    photoControls->addLayout(photoActions);

    auto* ocrOptions = new QHBoxLayout;
    ocrOptions->addWidget(new QLabel(tr("OCR language"), m_photoGroup));
    m_ocrLanguageCombo = new QComboBox(m_photoGroup);
    m_ocrLanguageCombo->setAccessibleName(tr("OCR language"));
    m_ocrLanguageCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_ocrLanguageCombo->setMinimumContentsLength(14);
    ocrOptions->addWidget(m_ocrLanguageCombo, 1);
    ocrOptions->addWidget(new QLabel(tr("Layout"), m_photoGroup));
    m_ocrLayoutCombo = new QComboBox(m_photoGroup);
    m_ocrLayoutCombo->addItem(tr("Automatic"), static_cast<int>(PhotoOcrLayout::Automatic));
    m_ocrLayoutCombo->addItem(tr("Single text block"),
                              static_cast<int>(PhotoOcrLayout::SingleBlock));
    m_ocrLayoutCombo->addItem(tr("Sparse text"),
                              static_cast<int>(PhotoOcrLayout::SparseText));
    const int storedLayout = m_ocrLayoutCombo->findData(m_settings->photoOcrLayout());
    if (storedLayout >= 0) {
        m_ocrLayoutCombo->setCurrentIndex(storedLayout);
    }
    ocrOptions->addWidget(m_ocrLayoutCombo);
    m_recognizePhotoButton = new QPushButton(tr("Recognize again"), m_photoGroup);
    ocrOptions->addWidget(m_recognizePhotoButton);
    photoControls->addLayout(ocrOptions);

    m_photoInfo = new QLabel(m_photoGroup);
    m_photoInfo->setWordWrap(true);
    m_photoInfo->setTextFormat(Qt::PlainText);
    m_photoInfo->setTextInteractionFlags(Qt::TextSelectableByMouse);
    photoControls->addWidget(m_photoInfo);
    photoRoot->addLayout(photoControls, 1);
    root->addWidget(m_photoGroup);

    auto* inputHeader = new QHBoxLayout;
    inputHeader->addWidget(new QLabel(tr("Text"), this));
    inputHeader->addStretch();
    m_counter = new QLabel(this);
    inputHeader->addWidget(m_counter);
    root->addLayout(inputHeader);

    m_input = new QPlainTextEdit(this);
    m_input->setPlaceholderText(tr("Enter or paste text to translate"));
    m_input->setMinimumHeight(100);
    m_input->setAcceptDrops(false);
    root->addWidget(m_input, 1);

    auto* actionRow = new QHBoxLayout;
    m_clearButton = new QPushButton(tr("Clear"), this);
    m_translateButton = new QPushButton(tr("Translate"), this);
    m_translateButton->setDefault(true);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_cancelButton->setVisible(false);
    actionRow->addWidget(m_clearButton);
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
    m_output->setMinimumHeight(100);
    root->addWidget(m_output, 1);

    m_status = new QLabel(this);
    m_status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_status->setWordWrap(true);
    root->addWidget(m_status);

    m_privacy = new QLabel(this);
    m_privacy->setWordWrap(true);
    root->addWidget(m_privacy);

    populateLanguages();
    populateOcrLanguages();
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
        if (m_ocrLanguageCombo->currentData().toString() == QStringLiteral("match-source")) {
            m_ocrLanguageCombo->setToolTip(
                tr("Will use installed OCR language: %1")
                    .arg(PhotoOcrEngine::languageDisplayName(currentOcrLanguage())));
        }
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
    connect(m_openPhotoButton, &QPushButton::clicked, this, &TranslationPanel::choosePhoto);
    connect(m_pastePhotoButton, &QPushButton::clicked, this, &TranslationPanel::pastePhoto);
    connect(m_recognizePhotoButton, &QPushButton::clicked, this,
            &TranslationPanel::recognizeCurrentPhoto);
    connect(m_ocrLanguageCombo, &QComboBox::currentIndexChanged, this, [this] {
        m_settings->setPhotoOcrLanguage(m_ocrLanguageCombo->currentData().toString());
        if (m_ocrLanguageCombo->currentData().toString() == QStringLiteral("match-source")) {
            m_ocrLanguageCombo->setToolTip(
                tr("Will use installed OCR language: %1")
                    .arg(PhotoOcrEngine::languageDisplayName(currentOcrLanguage())));
        } else {
            m_ocrLanguageCombo->setToolTip({});
        }
    });
    connect(m_ocrLayoutCombo, &QComboBox::currentIndexChanged, this, [this] {
        m_settings->setPhotoOcrLayout(m_ocrLayoutCombo->currentData().toInt());
    });
    connect(m_translateButton, &QPushButton::clicked, this,
            &TranslationPanel::requestTranslation);
    connect(m_cancelButton, &QPushButton::clicked, m_controller,
            &TranslationController::cancel);
    connect(m_clearButton, &QPushButton::clicked, this, [this] {
        m_input->clear();
        m_output->clear();
        m_status->clear();
        clearPhoto();
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
    connect(m_controller, &TranslationController::providerRequestStarted, this, [this] {
        m_status->setText(tr("Connecting to the selected model…"));
    });
    connect(m_controller, &TranslationController::openRouterStillRouting, this, [this] {
        m_status->setText(tr("OpenRouter is still selecting an available endpoint…"));
    });
    connect(m_controller, &TranslationController::freeRouteRetrying, this, [this] {
        m_status->setText(tr("The first free route was too slow; trying another one…"));
    });
    connect(m_controller, &TranslationController::modelProcessing, this, [this] {
        m_status->setText(tr("The model is processing the text…"));
    });
    connect(m_controller, &TranslationController::translationChunk, this,
            &TranslationPanel::appendChunk);
    connect(m_controller, &TranslationController::inferenceRouteChanged, this,
            [this] { updateProviderDisplay(); });
    connect(m_controller, &TranslationController::translationFinished, this,
            &TranslationPanel::finishRequest);
    connect(m_controller, &TranslationController::requestFailed, this,
            &TranslationPanel::showError);
    connect(m_ocrEngine, &PhotoOcrEngine::recognitionStarted, this, [this] {
        m_status->setText(tr("Recognizing text locally…"));
        m_photoInfo->setText(tr("Reading the image and running local OCR…"));
        setOcrBusy(true);
    });
    connect(m_ocrEngine, &PhotoOcrEngine::recognitionFinished, this,
            &TranslationPanel::handleOcrResult);
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

void TranslationPanel::dragEnterEvent(QDragEnterEvent* event) {
    if (!m_busy && !m_ocrBusy && event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (urls.size() == 1 && urls.first().isLocalFile()) {
            event->acceptProposedAction();
            return;
        }
    }
    QWidget::dragEnterEvent(event);
}

void TranslationPanel::dropEvent(QDropEvent* event) {
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!m_busy && !m_ocrBusy && urls.size() == 1 && urls.first().isLocalFile()) {
        m_photoPath = urls.first().toLocalFile();
        m_photoImage = {};
        m_photoPreview->clear();
        m_photoPreview->setVisible(false);
        recognizeCurrentPhoto();
        event->acceptProposedAction();
        return;
    }
    QWidget::dropEvent(event);
}

void TranslationPanel::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updatePhotoPreview();
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

void TranslationPanel::populateOcrLanguages() {
    m_ocrLanguageCombo->blockSignals(true);
    m_ocrLanguageCombo->clear();
    m_ocrLanguageCombo->addItem(tr("Match translation source (recommended)"),
                                QStringLiteral("match-source"));

    const QStringList& installed = m_ocrEngine->availableLanguages();
    QStringList common;
    for (const QString& code : {QStringLiteral("eng"), QStringLiteral("deu"),
                                QStringLiteral("rus"), QStringLiteral("ukr")}) {
        if (installed.contains(code)) {
            common.append(code);
        }
    }
    if (common.size() >= 2) {
        const QString combined = common.join(QChar('+'));
        m_ocrLanguageCombo->addItem(
            tr("Mixed: %1").arg(PhotoOcrEngine::languageDisplayName(combined)), combined);
    }
    for (const QString& code : installed) {
        if (code == QStringLiteral("osd") || code == QStringLiteral("equ")) {
            continue;
        }
        m_ocrLanguageCombo->addItem(
            QStringLiteral("%1 · %2").arg(PhotoOcrEngine::languageDisplayName(code), code), code);
    }

    const QString stored = m_settings->photoOcrLanguage();
    const int storedIndex = m_ocrLanguageCombo->findData(stored);
    m_ocrLanguageCombo->setCurrentIndex(storedIndex >= 0 ? storedIndex : 0);
    m_ocrLanguageCombo->blockSignals(false);

    if (installed.isEmpty()) {
        m_photoInfo->setText(tr("No OCR language data is installed. Install a Tesseract language pack."));
    } else {
        m_photoInfo->setText(
            tr("Open, paste, or drop an image. Recognition stays on this device."));
    }
    if (m_ocrLanguageCombo->currentData().toString() == QStringLiteral("match-source")) {
        m_ocrLanguageCombo->setToolTip(
            tr("Will use installed OCR language: %1")
                .arg(PhotoOcrEngine::languageDisplayName(currentOcrLanguage())));
    }
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
    const bool idle = !m_busy && !m_ocrBusy;
    m_translateButton->setEnabled(idle && hasInput && withinLimit && pairValid);
    m_swapButton->setEnabled(idle);
    m_sourceCombo->setEnabled(idle);
    m_targetCombo->setEnabled(idle);
    m_styleCombo->setEnabled(idle);
    m_openPhotoButton->setEnabled(idle && !m_ocrEngine->availableLanguages().isEmpty());
    m_pastePhotoButton->setEnabled(idle && !m_ocrEngine->availableLanguages().isEmpty());
    m_ocrLanguageCombo->setEnabled(idle && !m_ocrEngine->availableLanguages().isEmpty());
    m_ocrLayoutCombo->setEnabled(idle && !m_ocrEngine->availableLanguages().isEmpty());
    m_recognizePhotoButton->setEnabled(
        idle && (!m_photoPath.isEmpty() || !m_photoImage.isNull()) &&
        !m_ocrEngine->availableLanguages().isEmpty());
    m_clearButton->setEnabled(idle);
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

    QString providerPrivacy;
    if (provider.openRouter && provider.zeroDataRetention) {
        providerPrivacy = tr("Strict route: Zero Data Retention endpoints only");
    } else if (provider.openRouter && provider.denyDataCollection) {
        providerPrivacy = tr("Private route: providers that collect prompts are excluded");
    } else {
        providerPrivacy = tr("Provider policy applies; review it before sending sensitive text");
    }
    m_privacy->setText(
        providerPrivacy + QChar('\n') +
        tr("Photo OCR is local. Only extracted text is sent after you press Translate."));
}

void TranslationPanel::updatePhotoPreview() {
    if (m_photoImage.isNull() || !m_photoPreview->isVisible()) {
        return;
    }
    const QSize target = m_photoPreview->contentsRect().size() - QSize(8, 8);
    m_photoPreview->setPixmap(
        QPixmap::fromImage(m_photoImage)
            .scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

void TranslationPanel::choosePhoto() {
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Choose a photo"), {},
        tr("Images (*.png *.jpg *.jpeg *.webp *.bmp *.tif *.tiff);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    m_photoPath = path;
    m_photoImage = {};
    m_photoPreview->clear();
    m_photoPreview->setVisible(false);
    recognizeCurrentPhoto();
}

void TranslationPanel::pastePhoto() {
    const QMimeData* mime = QApplication::clipboard()->mimeData();
    if (!mime || !mime->hasImage()) {
        m_status->setText(tr("The clipboard does not contain an image."));
        return;
    }
    QImage image = qvariant_cast<QImage>(mime->imageData());
    if (image.isNull()) {
        const QPixmap pixmap = qvariant_cast<QPixmap>(mime->imageData());
        image = pixmap.toImage();
    }
    if (image.isNull()) {
        m_status->setText(tr("The clipboard image could not be read."));
        return;
    }
    m_photoPath.clear();
    m_photoImage = image;
    m_photoPreview->clear();
    m_photoPreview->setVisible(false);
    const QString language = currentOcrLanguage();
    if (language.isEmpty()) {
        m_status->setText(tr("No suitable OCR language is installed."));
        return;
    }
    const auto layout =
        static_cast<PhotoOcrLayout>(m_ocrLayoutCombo->currentData().toInt());
    m_ocrEngine->recognizeImage(m_photoImage, tr("Clipboard image"), language, layout);
}

void TranslationPanel::recognizeCurrentPhoto() {
    if (m_busy || m_ocrBusy || (m_photoPath.isEmpty() && m_photoImage.isNull())) {
        return;
    }
    const QString language = currentOcrLanguage();
    if (language.isEmpty()) {
        m_status->setText(tr("No suitable OCR language is installed."));
        return;
    }
    const auto layout =
        static_cast<PhotoOcrLayout>(m_ocrLayoutCombo->currentData().toInt());
    if (!m_photoPath.isEmpty()) {
        m_ocrEngine->recognizeFile(m_photoPath, language, layout);
    } else {
        m_ocrEngine->recognizeImage(m_photoImage, tr("Clipboard image"), language, layout);
    }
}

void TranslationPanel::handleOcrResult(const PhotoOcrResult& result) {
    setOcrBusy(false);
    if (!result.succeeded()) {
        m_status->setText(PhotoOcrEngine::errorMessage(result.error, result.detail));
        if (m_photoImage.isNull()) {
            m_photoInfo->setText(
                tr("Open, paste, or drop an image. Recognition stays on this device."));
        }
        return;
    }

    m_photoImage = result.image;
    m_photoPreview->setVisible(true);
    updatePhotoPreview();

    const qsizetype limit = m_settings->maximumInputCharacters();
    const bool truncated = result.outputTruncated || result.text.size() > limit;
    m_input->setPlainText(result.text.left(limit));
    m_photoInfo->setText(
        tr("%1 · %2 × %3 · %4 · confidence %5%")
            .arg(result.sourceLabel)
            .arg(result.sourceSize.width())
            .arg(result.sourceSize.height())
            .arg(PhotoOcrEngine::languageDisplayName(result.language))
            .arg(result.confidence));
    if (truncated) {
        m_status->setText(
            tr("Text was recognized locally and truncated to the configured input limit. Review it before translating."));
    } else if (result.confidence < 60) {
        m_status->setText(
            tr("Text was recognized locally with low confidence. Check the OCR language, layout, and extracted text before translating."));
    } else {
        m_status->setText(
            tr("Text recognized locally (%1% confidence). Review it, then press Translate.")
                .arg(result.confidence));
    }
    focusInput();
}

void TranslationPanel::clearPhoto() {
    m_photoPath.clear();
    m_photoImage = {};
    m_photoPreview->clear();
    m_photoPreview->setVisible(false);
    if (m_ocrEngine->availableLanguages().isEmpty()) {
        m_photoInfo->setText(tr("No OCR language data is installed. Install a Tesseract language pack."));
    } else {
        m_photoInfo->setText(
            tr("Open, paste, or drop an image. Recognition stays on this device."));
    }
    updateControls();
}

void TranslationPanel::beginRequest() {
    m_output->clear();
    m_status->setText(tr("Reading the saved API key…"));
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
    m_input->setReadOnly(m_busy || m_ocrBusy);
    updateControls();
}

void TranslationPanel::setOcrBusy(bool busy) {
    m_ocrBusy = busy;
    m_input->setReadOnly(m_busy || m_ocrBusy);
    updateControls();
}

QString TranslationPanel::currentSourceCode() const {
    return m_sourceCombo->currentData().toString();
}

QString TranslationPanel::currentTargetCode() const {
    return m_targetCombo->currentData().toString();
}

QString TranslationPanel::currentOcrLanguage() const {
    const QString selected = m_ocrLanguageCombo->currentData().toString();
    if (selected != QStringLiteral("match-source")) {
        return selected;
    }
    return PhotoOcrEngine::preferredLanguage(m_ocrEngine->availableLanguages(),
                                             currentSourceCode(),
                                             m_settings->interfaceLanguage());
}

void TranslationPanel::selectCode(QComboBox* combo, const QString& code) {
    const int index = combo->findData(code);
    if (index >= 0) {
        combo->setCurrentIndex(index);
    }
}

} // namespace verbuno
