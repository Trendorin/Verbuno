#include "core/EndpointValidator.h"

#include <QHostAddress>

namespace translunix {

EndpointValidation EndpointValidator::validate(const QUrl& endpoint) {
    if (!endpoint.isValid() || endpoint.isEmpty()) {
        return {false, QStringLiteral("The provider endpoint is not a valid URL.")};
    }
    if (!endpoint.userInfo().isEmpty()) {
        return {false, QStringLiteral("Credentials must not be embedded in the endpoint URL.")};
    }
    if (!endpoint.fragment().isEmpty() || endpoint.hasQuery()) {
        return {false, QStringLiteral("The endpoint must not contain a query or fragment.")};
    }
    if (endpoint.host().trimmed().isEmpty()) {
        return {false, QStringLiteral("The provider endpoint has no host.")};
    }

    const QString scheme = endpoint.scheme().toLower();
    if (scheme == QStringLiteral("https")) {
        return {true, {}};
    }
    if (scheme == QStringLiteral("http") && isLoopbackHost(endpoint.host())) {
        return {true, {}};
    }
    return {false,
            QStringLiteral("Use HTTPS. Plain HTTP is allowed only for a loopback provider.")};
}

bool EndpointValidator::isOpenRouter(const QUrl& endpoint) {
    return endpoint.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0 &&
           endpoint.host().compare(QStringLiteral("openrouter.ai"), Qt::CaseInsensitive) == 0 &&
           endpoint.path().startsWith(QStringLiteral("/api/v1/"));
}

bool EndpointValidator::isLoopbackHost(const QString& host) {
    if (host.compare(QStringLiteral("localhost"), Qt::CaseInsensitive) == 0) {
        return true;
    }
    QHostAddress address;
    return address.setAddress(host) && address.isLoopback();
}

} // namespace translunix
