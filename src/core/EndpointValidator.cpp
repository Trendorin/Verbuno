#include "core/EndpointValidator.h"

#include <QCoreApplication>
#include <QHostAddress>

namespace translunix {

namespace {
QString uiText(const char* source) {
    return QCoreApplication::translate("translunix::EndpointValidator", source);
}
}

EndpointValidation EndpointValidator::validate(const QUrl& endpoint) {
    if (!endpoint.isValid() || endpoint.isEmpty()) {
        return {false, uiText("The provider endpoint is not a valid URL.")};
    }
    if (!endpoint.userInfo().isEmpty()) {
        return {false, uiText("Credentials must not be embedded in the endpoint URL.")};
    }
    if (!endpoint.fragment().isEmpty() || endpoint.hasQuery()) {
        return {false, uiText("The endpoint must not contain a query or fragment.")};
    }
    if (endpoint.host().trimmed().isEmpty()) {
        return {false, uiText("The provider endpoint has no host.")};
    }

    const QString scheme = endpoint.scheme().toLower();
    if (scheme == QStringLiteral("https")) {
        return {true, {}};
    }
    if (scheme == QStringLiteral("http") && isLoopbackHost(endpoint.host())) {
        return {true, {}};
    }
    return {false, uiText("Use HTTPS. Plain HTTP is allowed only for a loopback provider.")};
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
