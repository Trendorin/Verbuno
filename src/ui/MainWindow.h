#pragma once

#include <QMainWindow>

class QListWidget;
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

signals:
    void settingsRequested();
    void hiddenToTray();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    AppSettings* m_settings;
    bool m_trayAvailable = false;
    QListWidget* m_navigation = nullptr;
    QStackedWidget* m_pages = nullptr;
    TranslationPanel* m_translator = nullptr;
    HistoryPage* m_history = nullptr;
};

} // namespace translunix
