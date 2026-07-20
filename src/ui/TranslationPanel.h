#pragma once

#include <QImage>
#include <QWidget>

class QComboBox;
class QDragEnterEvent;
class QDropEvent;
class QGroupBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QResizeEvent;
class QToolButton;

namespace verbuno {

class AppSettings;
class PhotoOcrEngine;
struct PhotoOcrResult;
class TranslationController;

class TranslationPanel final : public QWidget {
    Q_OBJECT

public:
    TranslationPanel(TranslationController* controller,
                     AppSettings* settings,
                     QWidget* parent = nullptr);

    void focusInput();
    [[nodiscard]] QString inputText() const;
    void setInputText(const QString& text);
    [[nodiscard]] QString outputText() const;
    void setOutputText(const QString& text);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void populateLanguages();
    void populateOcrLanguages();
    void updateCounter();
    void updateControls();
    void updateProviderDisplay();
    void updatePhotoPreview();
    void requestTranslation();
    void swapLanguages();
    void choosePhoto();
    void pastePhoto();
    void recognizeCurrentPhoto();
    void handleOcrResult(const PhotoOcrResult& result);
    void clearPhoto();
    void beginRequest();
    void appendChunk(const QString& chunk);
    void finishRequest(const QString& result);
    void showError(const QString& message);
    void setBusy(bool busy);
    void setOcrBusy(bool busy);
    [[nodiscard]] QString currentSourceCode() const;
    [[nodiscard]] QString currentTargetCode() const;
    [[nodiscard]] QString currentOcrLanguage() const;
    static void selectCode(QComboBox* combo, const QString& code);

    TranslationController* m_controller;
    AppSettings* m_settings;
    PhotoOcrEngine* m_ocrEngine;
    bool m_busy = false;
    bool m_ocrBusy = false;
    QString m_photoPath;
    QImage m_photoImage;

    QComboBox* m_sourceCombo = nullptr;
    QComboBox* m_targetCombo = nullptr;
    QComboBox* m_styleCombo = nullptr;
    QComboBox* m_ocrLanguageCombo = nullptr;
    QComboBox* m_ocrLayoutCombo = nullptr;
    QPlainTextEdit* m_input = nullptr;
    QPlainTextEdit* m_output = nullptr;
    QLabel* m_counter = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_providerSummary = nullptr;
    QLabel* m_privacy = nullptr;
    QLabel* m_photoInfo = nullptr;
    QLabel* m_photoPreview = nullptr;
    QGroupBox* m_photoGroup = nullptr;
    QPushButton* m_openPhotoButton = nullptr;
    QPushButton* m_pastePhotoButton = nullptr;
    QPushButton* m_recognizePhotoButton = nullptr;
    QPushButton* m_clearButton = nullptr;
    QPushButton* m_translateButton = nullptr;
    QPushButton* m_cancelButton = nullptr;
    QToolButton* m_swapButton = nullptr;
};

} // namespace verbuno
