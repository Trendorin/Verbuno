#pragma once

#include <QString>
#include <QUrl>

namespace translunix {

struct EndpointValidation {
    bool valid = false;
    QString error;
};

class EndpointValidator final {
public:
    [[nodiscard]] static EndpointValidation validate(const QUrl& endpoint);
    [[nodiscard]] static bool isOpenRouter(const QUrl& endpoint);
    [[nodiscard]] static bool isLoopbackHost(const QString& host);
};

} // namespace translunix
