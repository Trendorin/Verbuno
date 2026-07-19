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
    QCoreApplication::setApplicationName(QStringLiteral("TranslUnix"));
    QCoreApplication::setApplicationVersion(QStringLiteral(TRANSLUNIX_VERSION));
    QGuiApplication::setDesktopFileName(QStringLiteral("io.github.trendorin.TranslUnix"));
    application.setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));
    application.setQuitOnLastWindowClosed(false);

    const QStringList arguments = application.arguments().mid(1);
    if (arguments.contains(QStringLiteral("--version"))) {
        QTextStream(stdout) << "TranslUnix " << TRANSLUNIX_VERSION << '\n';
        return 0;
    }
    if (arguments.contains(QStringLiteral("--help")) ||
        arguments.contains(QStringLiteral("-h"))) {
        QTextStream(stdout)
            << "Usage: translunix [--toggle|--show|--history|--settings|--quit|--version]\n";
        return 0;
    }
    if (arguments.contains(QStringLiteral("--check-translations"))) {
        translunix::InterfaceLanguageManager languageManager;
        const QString source = QStringLiteral("Settings");
        for (const QString& code : {QStringLiteral("ru"), QStringLiteral("uk"),
                                    QStringLiteral("de")}) {
            if (!languageManager.applyLanguage(code) ||
                QCoreApplication::translate("translunix::MainWindow", "Settings") == source) {
                QTextStream(stderr) << "Translation catalog check failed for " << code << '\n';
                return 1;
            }
        }
        QTextStream(stdout) << "Translation catalogs: OK\n";
        return 0;
    }

    translunix::SingleInstance instance(QStringLiteral(TRANSLUNIX_APP_ID), arguments);
    if (!instance.isPrimary()) {
        return 0;
    }

    translunix::TrayController controller;
    QObject::connect(&instance, &translunix::SingleInstance::argumentsReceived, &controller,
                     &translunix::TrayController::handleArguments);
    controller.start(arguments);
    return application.exec();
}
