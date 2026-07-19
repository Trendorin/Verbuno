#pragma once

#include <QLocalServer>
#include <QObject>
#include <QStringList>

namespace translunix {

class SingleInstance final : public QObject {
    Q_OBJECT

public:
    SingleInstance(const QString& applicationId,
                   const QStringList& arguments,
                   QObject* parent = nullptr);

    [[nodiscard]] bool isPrimary() const;

signals:
    void argumentsReceived(const QStringList& arguments);

private:
    bool forwardToPrimary(const QStringList& arguments);
    void acceptConnections();
    void parseMessage(const QByteArray& payload);

    QLocalServer m_server;
    QString m_serverName;
    bool m_primary = false;
};

} // namespace translunix
