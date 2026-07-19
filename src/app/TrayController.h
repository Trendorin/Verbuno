#pragma once

#include "core/AppSettings.h"
#include "core/HistoryStore.h"
#include "core/SecretStore.h"
#include "core/TranslationController.h"

#include <QObject>
#include <QPointer>
#include <QStringList>

class QSystemTrayIcon;

namespace translunix {

class MainWindow;
class SettingsDialog;
class TranslatorPopup;

class TrayController final : public QObject {
    Q_OBJECT

public:
    explicit TrayController(QObject* parent = nullptr);

    void start(const QStringList& arguments = {});
    void handleArguments(const QStringList& arguments);

private:
    void createTray();
    void showPopup();
    void showMain();
    void showHistory();
    void showSettings();
    void notifyHidden();

    AppSettings m_settings;
    SecretStore m_secretStore;
    HistoryStore m_history;
    TranslationController m_controller;
    MainWindow* m_mainWindow = nullptr;
    TranslatorPopup* m_popup = nullptr;
    QSystemTrayIcon* m_tray = nullptr;
    QPointer<SettingsDialog> m_settingsDialog;
    bool m_hiddenNoticeShown = false;
};

} // namespace translunix
