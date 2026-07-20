#include "app/SingleInstance.h"
#include "app/TrayController.h"
#include "core/InterfaceLanguageManager.h"

#include <QApplication>
#include <QIcon>
#include <QTextStream>

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Trendorin"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("github.com/Trendorin"));
    QCoreApplication::setApplicationName(QStringLiteral("Verbuno"));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERBUNO_VERSION));
    QGuiApplication::setDesktopFileName(QStringLiteral("io.github.trendorin.Verbuno"));
    application.setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));
    application.setQuitOnLastWindowClosed(false);

    const QStringList arguments = application.arguments().mid(1);
    if (arguments.contains(QStringLiteral("--version"))) {
        QTextStream(stdout) << "Verbuno " << VERBUNO_VERSION << '\n';
        return 0;
    }
    if (arguments.contains(QStringLiteral("--help")) ||
        arguments.contains(QStringLiteral("-h"))) {
        QTextStream(stdout)
            << "Usage: verbuno [--toggle|--show|--history|--settings|--quit|--version]\n";
        return 0;
    }
    if (arguments.contains(QStringLiteral("--check-translations"))) {
        verbuno::InterfaceLanguageManager languageManager;
        const QString source = QStringLiteral("Settings");
        for (const QString& code : {QStringLiteral("ru"), QStringLiteral("uk"),
                                    QStringLiteral("de")}) {
            if (!languageManager.applyLanguage(code) ||
                QCoreApplication::translate("verbuno::MainWindow", "Settings") == source ||
                QCoreApplication::translate("verbuno::TranslationPanel", "Text from photo") ==
                    QStringLiteral("Text from photo") ||
                QCoreApplication::translate("verbuno::PhotoOcrEngine",
                                            "No readable text was found. Try another OCR "
                                            "language or layout.") ==
                    QStringLiteral("No readable text was found. Try another OCR language or "
                                   "layout.")) {
                QTextStream(stderr) << "Translation catalog check failed for " << code << '\n';
                return 1;
            }
        }
        QTextStream(stdout) << "Translation catalogs: OK\n";
        return 0;
    }

    verbuno::SingleInstance instance(QStringLiteral(VERBUNO_APP_ID), arguments);
    if (!instance.isPrimary()) {
        return 0;
    }

    verbuno::TrayController controller;
    QObject::connect(&instance, &verbuno::SingleInstance::argumentsReceived, &controller,
                     &verbuno::TrayController::handleArguments);
    controller.start(arguments);
    return application.exec();
}
