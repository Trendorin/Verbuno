#pragma once

#include "core/AppSettings.h"
#include "core/HistoryStore.h"
#include "core/InterfaceLanguageManager.h"
#include "core/SecretStore.h"
#include "core/TranslationController.h"

#include <QObject>
#include <QPointer>
#include <QStringList>

class QSystemTrayIcon;
class QMenu;

namespace verbuno {

class MainWindow;
class SettingsDialog;

class TrayController final : public QObject {
    Q_OBJECT

public:
    explicit TrayController(QObject* parent = nullptr);
    ~TrayController() override;

    void start(const QStringList& arguments = {});
    void handleArguments(const QStringList& arguments);

private:
    void createMainWindow();
    void createTray();
    void createTrayMenu();
    void toggleMainWindow();
    void showMain();
    void showHistory();
    void showSettings();
    void notifyHidden();
    void applyInterfaceLanguage(const QString& languageCode);

    AppSettings m_settings;
    InterfaceLanguageManager m_languageManager;
    SecretStore m_secretStore;
    HistoryStore m_history;
    TranslationController m_controller;
    MainWindow* m_mainWindow = nullptr;
    QSystemTrayIcon* m_tray = nullptr;
    QMenu* m_trayMenu = nullptr;
    QPointer<SettingsDialog> m_settingsDialog;
    bool m_hiddenNoticeShown = false;
};

} // namespace verbuno
