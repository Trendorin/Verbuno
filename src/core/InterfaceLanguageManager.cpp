#include "core/InterfaceLanguageManager.h"

#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>

namespace verbuno {

InterfaceLanguageManager::InterfaceLanguageManager(QObject* parent)
    : QObject(parent) {
}

InterfaceLanguageManager::~InterfaceLanguageManager() {
    if (m_translator && QCoreApplication::instance()) {
        QCoreApplication::removeTranslator(m_translator.get());
    }
}

QString InterfaceLanguageManager::currentLanguage() const {
    return m_currentLanguage;
}

bool InterfaceLanguageManager::applyLanguage(const QString& languageCode) {
    const QString normalized = normalize(languageCode);
    if (normalized == m_currentLanguage) {
        return true;
    }

    std::unique_ptr<QTranslator> candidate;
    if (normalized != QStringLiteral("en")) {
        candidate = std::make_unique<QTranslator>();
        const QString resource =
            QStringLiteral(":/i18n/verbuno_%1.qm").arg(normalized);
        if (!candidate->load(resource)) {
            return false;
        }
    }

    if (m_translator) {
        QCoreApplication::removeTranslator(m_translator.get());
    }
    m_translator = std::move(candidate);
    if (m_translator) {
        QCoreApplication::installTranslator(m_translator.get());
    }
    m_currentLanguage = normalized;
    emit languageChanged(m_currentLanguage);
    return true;
}

QString InterfaceLanguageManager::normalize(const QString& languageCode) {
    QString normalized = languageCode.trimmed().toLower();
    const qsizetype separator = normalized.indexOf(QChar('-')) >= 0
                                    ? normalized.indexOf(QChar('-'))
                                    : normalized.indexOf(QChar('_'));
    if (separator > 0) {
        normalized.truncate(separator);
    }
    return supportedCodes().contains(normalized) ? normalized : QStringLiteral("en");
}

QString InterfaceLanguageManager::systemDefault() {
    switch (QLocale::system().language()) {
    case QLocale::Russian:
        return QStringLiteral("ru");
    case QLocale::Ukrainian:
        return QStringLiteral("uk");
    case QLocale::German:
        return QStringLiteral("de");
    default:
        return QStringLiteral("en");
    }
}

QStringList InterfaceLanguageManager::supportedCodes() {
    return {QStringLiteral("en"), QStringLiteral("ru"), QStringLiteral("uk"),
            QStringLiteral("de")};
}

} // namespace verbuno
