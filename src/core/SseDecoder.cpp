#include "core/SseDecoder.h"

#include <utility>

namespace translunix {

QVector<QByteArray> SseDecoder::feed(const QByteArray& bytes) {
    QVector<QByteArray> events;
    m_buffer.append(bytes);

    qsizetype newline = m_buffer.indexOf('\n');
    while (newline >= 0) {
        QByteArray line = m_buffer.left(newline);
        m_buffer.remove(0, newline + 1);
        consumeLine(std::move(line), events);
        newline = m_buffer.indexOf('\n');
    }
    return events;
}

QVector<QByteArray> SseDecoder::finish() {
    QVector<QByteArray> events;
    if (!m_buffer.isEmpty()) {
        consumeLine(std::move(m_buffer), events);
        m_buffer.clear();
    }
    finishEvent(events);
    return events;
}

void SseDecoder::reset() {
    m_buffer.clear();
    m_eventData.clear();
}

void SseDecoder::consumeLine(QByteArray line, QVector<QByteArray>& events) {
    if (line.endsWith('\r')) {
        line.chop(1);
    }
    if (line.isEmpty()) {
        finishEvent(events);
        return;
    }
    if (line.startsWith(':')) {
        return;
    }
    if (!line.startsWith("data:")) {
        return;
    }

    QByteArray value = line.mid(5);
    if (value.startsWith(' ')) {
        value.remove(0, 1);
    }
    if (!m_eventData.isEmpty()) {
        m_eventData.append('\n');
    }
    m_eventData.append(value);
}

void SseDecoder::finishEvent(QVector<QByteArray>& events) {
    if (!m_eventData.isEmpty()) {
        events.push_back(m_eventData);
        m_eventData.clear();
    }
}

} // namespace translunix
