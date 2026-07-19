#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>

class QTranslator;

namespace translunix {

class InterfaceLanguageManager final : public QObject {
    Q_OBJECT

public:
    explicit InterfaceLanguageManager(QObject* parent = nullptr);
    ~InterfaceLanguageManager() override;

    [[nodiscard]] QString currentLanguage() const;
    [[nodiscard]] bool applyLanguage(const QString& languageCode);

    [[nodiscard]] static QString normalize(const QString& languageCode);
    [[nodiscard]] static QString systemDefault();
    [[nodiscard]] static QStringList supportedCodes();

signals:
    void languageChanged(const QString& languageCode);

private:
    QString m_currentLanguage;
    std::unique_ptr<QTranslator> m_translator;
};

} // namespace translunix
