#include "app/SingleInstance.h"

#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QSharedPointer>

namespace translunix {

namespace {
constexpr qsizetype kMaximumMessageSize = 64 * 1024;
}

SingleInstance::SingleInstance(const QString& applicationId,
                               const QStringList& arguments,
                               QObject* parent)
    : QObject(parent) {
    const QByteArray digest =
        QCryptographicHash::hash(applicationId.toUtf8(), QCryptographicHash::Sha256).toHex();
    m_serverName = QStringLiteral("translunix-%1").arg(QString::fromLatin1(digest.left(20)));

    if (m_server.listen(m_serverName)) {
        m_primary = true;
        connect(&m_server, &QLocalServer::newConnection, this, &SingleInstance::acceptConnections);
        return;
    }
    if (forwardToPrimary(arguments)) {
        return;
    }

    QLocalServer::removeServer(m_serverName);
    m_primary = m_server.listen(m_serverName);
    if (m_primary) {
        connect(&m_server, &QLocalServer::newConnection, this, &SingleInstance::acceptConnections);
    }
}

bool SingleInstance::isPrimary() const {
    return m_primary;
}

bool SingleInstance::forwardToPrimary(const QStringList& arguments) {
    QLocalSocket socket;
    socket.connectToServer(m_serverName, QIODevice::WriteOnly);
    if (!socket.waitForConnected(500)) {
        return false;
    }

    QJsonArray array;
    for (const QString& argument : arguments) {
        array.append(argument);
    }
    const QByteArray payload = QJsonDocument(array).toJson(QJsonDocument::Compact);
    if (payload.size() > kMaximumMessageSize || socket.write(payload) != payload.size()) {
        return false;
    }
    if (!socket.waitForBytesWritten(500)) {
        return false;
    }
    socket.disconnectFromServer();
    return true;
}

void SingleInstance::acceptConnections() {
    while (m_server.hasPendingConnections()) {
        QLocalSocket* socket = m_server.nextPendingConnection();
        auto payload = QSharedPointer<QByteArray>::create();
        connect(socket, &QLocalSocket::readyRead, this, [socket, payload] {
            if (payload->size() < kMaximumMessageSize) {
                payload->append(socket->read(kMaximumMessageSize - payload->size()));
            } else {
                socket->readAll();
            }
        });
        connect(socket, &QLocalSocket::disconnected, this, [this, socket, payload] {
            parseMessage(*payload);
            socket->deleteLater();
        });
    }
}

void SingleInstance::parseMessage(const QByteArray& payload) {
    if (payload.isEmpty() || payload.size() > kMaximumMessageSize) {
        return;
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !document.isArray()) {
        return;
    }
    QStringList arguments;
    for (const QJsonValue& value : document.array()) {
        if (value.isString()) {
            arguments.push_back(value.toString());
        }
    }
    emit argumentsReceived(arguments);
}

} // namespace translunix
