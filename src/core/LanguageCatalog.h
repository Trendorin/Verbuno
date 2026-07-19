#pragma once

#include <QString>
#include <QVector>

#include <optional>

namespace translunix {

struct Language {
    QString code;
    QString name;
};

class LanguageCatalog final {
public:
    [[nodiscard]] static const QVector<Language>& all();
    [[nodiscard]] static std::optional<Language> byCode(const QString& code);
};

} // namespace translunix
