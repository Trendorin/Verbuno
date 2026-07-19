#include "app/TrayController.h"

#include "ui/MainWindow.h"
#include "ui/SettingsDialog.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>

namespace verbuno {

TrayController::TrayController(QObject* parent)
    : QObject(parent)
    , m_settings(this)
    , m_languageManager(this)
    , m_secretStore(this)
    , m_history({}, this)
    , m_controller(&m_settings, &m_secretStore, &m_history, this) {
    if (!m_languageManager.applyLanguage(m_settings.interfaceLanguage())) {
        m_languageManager.applyLanguage(QStringLiteral("en"));
    }
    createMainWindow();
}

TrayController::~TrayController() {
    if (m_tray) {
        m_tray->setContextMenu(nullptr);
    }
    delete m_trayMenu;
    delete m_mainWindow;
}

void TrayController::createMainWindow() {
    m_mainWindow = new MainWindow(&m_controller, &m_settings);
    connect(m_mainWindow, &MainWindow::settingsRequested, this, &TrayController::showSettings);
    connect(m_mainWindow, &MainWindow::hiddenToTray, this, &TrayController::notifyHidden);
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
    toggleMainWindow();
}

void TrayController::createTray() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }
    m_tray = new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/app.svg")), this);
    m_tray->setToolTip(QStringLiteral("Verbuno"));
    createTrayMenu();
    connect(m_tray, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger ||
                    reason == QSystemTrayIcon::DoubleClick) {
                    toggleMainWindow();
                }
            });
    m_tray->show();
}

void TrayController::createTrayMenu() {
    if (!m_tray) {
        return;
    }
    m_tray->setContextMenu(nullptr);
    delete m_trayMenu;
    m_trayMenu = new QMenu(m_mainWindow);
    QAction* openAction = m_trayMenu->addAction(tr("Open Verbuno"));
    QAction* historyAction = m_trayMenu->addAction(tr("Local history"));
    QAction* settingsAction = m_trayMenu->addAction(tr("Settings"));
    m_trayMenu->addSeparator();
    QAction* quitAction = m_trayMenu->addAction(tr("Quit"));
    m_tray->setContextMenu(m_trayMenu);
    connect(openAction, &QAction::triggered, this, &TrayController::showMain);
    connect(historyAction, &QAction::triggered, this, &TrayController::showHistory);
    connect(settingsAction, &QAction::triggered, this, &TrayController::showSettings);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void TrayController::toggleMainWindow() {
    if (m_mainWindow->isVisible() && m_mainWindow->isActiveWindow() &&
        !m_mainWindow->isMinimized()) {
        m_mainWindow->hide();
        return;
    }
    showMain();
}

void TrayController::showMain() {
    m_mainWindow->showTranslator();
}

void TrayController::showHistory() {
    m_mainWindow->showHistory();
}

void TrayController::showSettings() {
    if (m_settingsDialog) {
        if (m_settingsDialog->isMinimized()) {
            m_settingsDialog->showNormal();
        } else {
            m_settingsDialog->show();
        }
        m_settingsDialog->raise();
        m_settingsDialog->activateWindow();
        return;
    }
    m_settingsDialog = new SettingsDialog(&m_settings, &m_controller, m_mainWindow);
    m_settingsDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_settingsDialog, &SettingsDialog::interfaceLanguageChanged, this,
            &TrayController::applyInterfaceLanguage);
    m_settingsDialog->show();
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
}

void TrayController::applyInterfaceLanguage(const QString& languageCode) {
    QTimer::singleShot(0, this, [this, languageCode] {
        const bool wasVisible = m_mainWindow->isVisible();
        const bool wasShowingHistory = m_mainWindow->isShowingHistory();
        const QByteArray geometry = m_mainWindow->saveGeometry();
        const QString input = m_mainWindow->translatorInput();
        const QString output = m_mainWindow->translatorOutput();

        if (!m_languageManager.applyLanguage(languageCode)) {
            return;
        }

        if (m_tray) {
            m_tray->setContextMenu(nullptr);
        }
        delete m_trayMenu;
        m_trayMenu = nullptr;
        delete m_mainWindow;
        m_mainWindow = nullptr;
        m_settingsDialog = nullptr;

        createMainWindow();
        m_mainWindow->setTrayAvailable(m_tray != nullptr);
        m_mainWindow->restoreGeometry(geometry);
        m_mainWindow->setTranslatorInput(input);
        m_mainWindow->setTranslatorOutput(output);
        createTrayMenu();

        if (wasVisible) {
            if (wasShowingHistory) {
                showHistory();
            } else {
                showMain();
            }
        }
    });
}

void TrayController::notifyHidden() {
    if (!m_tray || m_hiddenNoticeShown) {
        return;
    }
    m_hiddenNoticeShown = true;
    m_tray->showMessage(QStringLiteral("Verbuno"),
                        tr("Verbuno is still running in the system tray."),
                        QSystemTrayIcon::Information, 3500);
}

} // namespace verbuno
