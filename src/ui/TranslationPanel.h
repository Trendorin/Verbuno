#pragma once

#include <QWidget>

class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QToolButton;

namespace translunix {

class AppSettings;
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
    [[nodiscard]] QString contextText() const;
    void setContextText(const QString& text);

private:
    void populateLanguages();
    void updateCounter();
    void updateControls();
    void updateProviderDisplay();
    void requestTranslation();
    void swapLanguages();
    void beginRequest();
    void appendChunk(const QString& chunk);
    void finishRequest(const QString& result);
    void showError(const QString& message);
    void setBusy(bool busy);
    [[nodiscard]] QString currentSourceCode() const;
    [[nodiscard]] QString currentTargetCode() const;
    static void selectCode(QComboBox* combo, const QString& code);

    TranslationController* m_controller;
    AppSettings* m_settings;
    bool m_busy = false;

    QComboBox* m_sourceCombo = nullptr;
    QComboBox* m_targetCombo = nullptr;
    QComboBox* m_styleCombo = nullptr;
    QPlainTextEdit* m_input = nullptr;
    QPlainTextEdit* m_output = nullptr;
    QPlainTextEdit* m_context = nullptr;
    QLabel* m_counter = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_providerSummary = nullptr;
    QLabel* m_privacy = nullptr;
    QPushButton* m_translateButton = nullptr;
    QPushButton* m_cancelButton = nullptr;
    QToolButton* m_swapButton = nullptr;
};

} // namespace translunix
