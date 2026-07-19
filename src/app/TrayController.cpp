#include "app/TrayController.h"

#include "ui/MainWindow.h"
#include "ui/SettingsDialog.h"
#include "ui/TranslatorPopup.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>

namespace translunix {

TrayController::TrayController(QObject* parent)
    : QObject(parent)
    , m_settings(this)
    , m_secretStore(this)
    , m_history({}, this)
    , m_controller(&m_settings, &m_secretStore, &m_history, this)
    , m_mainWindow(new MainWindow(&m_controller, &m_settings))
    , m_popup(new TranslatorPopup(&m_controller, &m_settings)) {
    connect(m_mainWindow, &MainWindow::settingsRequested, this, &TrayController::showSettings);
    connect(m_mainWindow, &MainWindow::hiddenToTray, this, &TrayController::notifyHidden);
    connect(m_popup, &TranslatorPopup::settingsRequested, this, &TrayController::showSettings);
    connect(m_popup, &TranslatorPopup::fullWindowRequested, this, &TrayController::showMain);
}

void TrayController::start(const QStringList& arguments) {
    createTray();
    m_mainWindow->setTrayAvailable(m_tray != nullptr);

    if (!arguments.isEmpty()) {
        handleArguments(arguments);
        return;
    }
    if (!m_tray || !m_settings.startInTray()) {
        showMain();
    }
}

void TrayController::handleArguments(const QStringList& arguments) {
    if (arguments.contains(QStringLiteral("--quit"))) {
        QApplication::quit();
        return;
    }
    if (arguments.contains(QStringLiteral("--settings"))) {
        showSettings();
        return;
    }
    if (arguments.contains(QStringLiteral("--history"))) {
        showHistory();
        return;
    }
    if (arguments.contains(QStringLiteral("--show"))) {
        showMain();
        return;
    }
    showPopup();
}

void TrayController::createTray() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }
    m_tray = new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/app.svg")), this);
    m_tray->setToolTip(QStringLiteral("TranslUnix"));
    auto* menu = new QMenu(m_mainWindow);
    QAction* quickAction = menu->addAction(tr("Quick translate"));
    QAction* openAction = menu->addAction(tr("Open TranslUnix"));
    QAction* historyAction = menu->addAction(tr("Local history"));
    QAction* settingsAction = menu->addAction(tr("Settings"));
    menu->addSeparator();
    QAction* quitAction = menu->addAction(tr("Quit"));
    m_tray->setContextMenu(menu);
    connect(quickAction, &QAction::triggered, this, &TrayController::showPopup);
    connect(openAction, &QAction::triggered, this, &TrayController::showMain);
    connect(historyAction, &QAction::triggered, this, &TrayController::showHistory);
    connect(settingsAction, &QAction::triggered, this, &TrayController::showSettings);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(m_tray, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger ||
                    reason == QSystemTrayIcon::DoubleClick) {
                    m_popup->toggle(m_tray->geometry());
                }
            });
    m_tray->show();
}

void TrayController::showPopup() {
    if (!m_tray) {
        showMain();
        return;
    }
    m_mainWindow->hide();
    m_popup->showAnimated(m_tray->geometry());
}

void TrayController::showMain() {
    m_popup->hide();
    m_mainWindow->showTranslator();
}

void TrayController::showHistory() {
    m_popup->hide();
    m_mainWindow->showHistory();
}

void TrayController::showSettings() {
    if (m_settingsDialog) {
        m_settingsDialog->show();
        m_settingsDialog->raise();
        m_settingsDialog->activateWindow();
        return;
    }
    m_settingsDialog = new SettingsDialog(&m_settings, &m_controller, m_mainWindow);
    m_settingsDialog->setAttribute(Qt::WA_DeleteOnClose);
    m_settingsDialog->show();
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
}

void TrayController::notifyHidden() {
    if (!m_tray || m_hiddenNoticeShown) {
        return;
    }
    m_hiddenNoticeShown = true;
    m_tray->showMessage(QStringLiteral("TranslUnix"),
                        tr("TranslUnix is still running in the system tray."),
                        QSystemTrayIcon::Information, 3500);
}

} // namespace translunix
