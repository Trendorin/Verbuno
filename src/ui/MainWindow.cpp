#include "ui/MainWindow.h"

#include "core/AppSettings.h"
#include "core/TranslationController.h"
#include "ui/AboutDialog.h"
#include "ui/HistoryPage.h"
#include "ui/TranslationPanel.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QStackedWidget>
#include <QStatusBar>
#include <QSizePolicy>
#include <QToolBar>

namespace verbuno {

MainWindow::MainWindow(TranslationController* controller,
                       AppSettings* settings,
                       QWidget* parent)
    : QMainWindow(parent)
    , m_controller(controller)
    , m_settings(settings) {
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                   Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                   Qt::WindowCloseButtonHint);
    setWindowTitle(QStringLiteral("Verbuno"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));
    setMinimumSize(820, 620);
    resize(1040, 760);

    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_navigation = new QListWidget(central);
    m_navigation->setFixedWidth(170);
    m_navigation->setFrameShape(QFrame::NoFrame);
    m_navigation->addItem(tr("Translate"));
    m_navigation->addItem(tr("Local history"));
    m_navigation->setCurrentRow(0);
    layout->addWidget(m_navigation);

    m_pages = new QStackedWidget(central);
    m_translator = new TranslationPanel(controller, settings, m_pages);
    m_history = new HistoryPage(controller, m_pages);
    m_pages->addWidget(m_translator);
    m_pages->addWidget(m_history);
    layout->addWidget(m_pages, 1);
    setCentralWidget(central);

    auto* settingsAction = new QAction(tr("Settings"), this);
    settingsAction->setShortcut(QKeySequence::Preferences);
    auto* aboutAction = new QAction(tr("About Verbuno"), this);
    auto* quitAction = new QAction(tr("Quit"), this);
    quitAction->setShortcut(QKeySequence::Quit);

    auto* applicationMenu = menuBar()->addMenu(tr("Application"));
    applicationMenu->addAction(settingsAction);
    applicationMenu->addAction(aboutAction);
    applicationMenu->addSeparator();
    applicationMenu->addAction(quitAction);

    auto* toolbar = addToolBar(tr("Main"));
    toolbar->setMovable(false);
    toolbar->addAction(settingsAction);
    toolbar->addSeparator();
    m_providerIndicator = new QLabel(toolbar);
    m_providerIndicator->setTextFormat(Qt::PlainText);
    m_providerIndicator->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_providerIndicator->setMinimumWidth(240);
    m_providerIndicator->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    toolbar->addWidget(m_providerIndicator);
    updateProviderIndicator();

    statusBar()->showMessage(
        tr("No telemetry · text is sent only to the configured provider endpoint"));

    connect(m_navigation, &QListWidget::currentRowChanged, m_pages,
            &QStackedWidget::setCurrentIndex);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::settingsRequested);
    connect(aboutAction, &QAction::triggered, this, [this] {
        AboutDialog dialog(this);
        dialog.exec();
    });
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(m_settings, &AppSettings::changed, this, &MainWindow::updateProviderIndicator);
    connect(m_controller, &TranslationController::inferenceRouteChanged, this,
            &MainWindow::updateProviderIndicator);
    connect(m_controller, &TranslationController::requestStarted, this,
            &MainWindow::updateProviderIndicator);
    connect(m_controller, &TranslationController::translationFinished, this,
            &MainWindow::updateProviderIndicator);
    connect(m_controller, &TranslationController::requestFailed, this,
            &MainWindow::updateProviderIndicator);
}

void MainWindow::setTrayAvailable(bool available) {
    m_trayAvailable = available;
}

void MainWindow::showTranslator() {
    m_navigation->setCurrentRow(0);
    if (isMinimized()) {
        showNormal();
    } else {
        show();
    }
    raise();
    activateWindow();
    m_translator->focusInput();
}

void MainWindow::showHistory() {
    m_navigation->setCurrentRow(1);
    if (isMinimized()) {
        showNormal();
    } else {
        show();
    }
    raise();
    activateWindow();
}

bool MainWindow::isShowingHistory() const {
    return m_navigation->currentRow() == 1;
}

QString MainWindow::translatorInput() const {
    return m_translator->inputText();
}

void MainWindow::setTranslatorInput(const QString& text) {
    m_translator->setInputText(text);
}

QString MainWindow::translatorOutput() const {
    return m_translator->outputText();
}

void MainWindow::setTranslatorOutput(const QString& text) {
    m_translator->setOutputText(text);
}

void MainWindow::updateProviderIndicator() {
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
    m_providerIndicator->setText(
        tr("Provider: %1 · Actual model: %2").arg(actualProvider, actualModel));
    m_providerIndicator->setToolTip(
        tr("Requested model or route: %1\nEndpoint: %2")
            .arg(provider.model.trimmed(),
                 provider.chatEndpoint.toString(QUrl::FullyEncoded)));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_trayAvailable && m_settings->closeToTray()) {
        hide();
        event->ignore();
        emit hiddenToTray();
        return;
    }
    QMainWindow::closeEvent(event);
}

} // namespace verbuno
