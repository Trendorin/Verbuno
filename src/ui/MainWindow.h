#pragma once

#include <QMainWindow>

class QListWidget;
class QLabel;
class QStackedWidget;

namespace translunix {

class AppSettings;
class HistoryPage;
class TranslationController;
class TranslationPanel;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(TranslationController* controller,
               AppSettings* settings,
               QWidget* parent = nullptr);

    void setTrayAvailable(bool available);
    void showTranslator();
    void showHistory();
    [[nodiscard]] bool isShowingHistory() const;
    [[nodiscard]] QString translatorInput() const;
    void setTranslatorInput(const QString& text);
    [[nodiscard]] QString translatorOutput() const;
    void setTranslatorOutput(const QString& text);
    [[nodiscard]] QString translatorContext() const;
    void setTranslatorContext(const QString& text);

signals:
    void settingsRequested();
    void hiddenToTray();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void updateProviderIndicator();

    AppSettings* m_settings;
    bool m_trayAvailable = false;
    QListWidget* m_navigation = nullptr;
    QStackedWidget* m_pages = nullptr;
    TranslationPanel* m_translator = nullptr;
    HistoryPage* m_history = nullptr;
    QLabel* m_providerIndicator = nullptr;
};

} // namespace translunix
