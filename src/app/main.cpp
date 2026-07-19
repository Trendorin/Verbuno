#include "app/SingleInstance.h"
#include "app/TrayController.h"

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QTextStream>
#include <QTranslator>

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

    QTranslator translator;
    if (QLocale::system().language() == QLocale::Russian &&
        translator.load(QStringLiteral(":/i18n/translunix_ru.qm"))) {
        application.installTranslator(&translator);
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
