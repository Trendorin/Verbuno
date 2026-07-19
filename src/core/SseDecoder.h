#pragma once

#include <QByteArray>
#include <QVector>

namespace translunix {

class SseDecoder final {
public:
    [[nodiscard]] QVector<QByteArray> feed(const QByteArray& bytes);
    [[nodiscard]] QVector<QByteArray> finish();
    void reset();

private:
    void consumeLine(QByteArray line, QVector<QByteArray>& events);
    void finishEvent(QVector<QByteArray>& events);

    QByteArray m_buffer;
    QByteArray m_eventData;
};

} // namespace translunix
