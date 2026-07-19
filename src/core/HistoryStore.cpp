#include "core/HistoryStore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>

#include <algorithm>
#include <utility>

namespace translunix {

namespace {
constexpr qint64 kMaximumHistoryBytes = 8 * 1024 * 1024;
}

HistoryStore::HistoryStore(QString storagePath, QObject* parent)
    : QObject(parent)
    , m_path(std::move(storagePath)) {
    if (m_path.isEmpty()) {
        m_path = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                     .filePath(QStringLiteral("history.json"));
    }
}

void HistoryStore::setEnabled(bool enabled) {
    m_enabled = enabled;
    if (m_enabled && !m_loaded) {
        load();
    }
}

bool HistoryStore::isEnabled() const {
    return m_enabled;
}

void HistoryStore::setPolicy(int maximumRecords, int retentionDays) {
    m_maximumRecords = std::clamp(maximumRecords, 10, 1000);
    m_retentionDays = std::clamp(retentionDays, 1, 365);
    if (m_loaded) {
        prune();
        if (m_enabled) {
            save();
        }
    }
}

void HistoryStore::append(TranslationRecord record) {
    if (!m_enabled) {
        return;
    }
    if (!m_loaded) {
        load();
    }
    if (record.id.isEmpty()) {
        record.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    if (!record.createdAt.isValid()) {
        record.createdAt = QDateTime::currentDateTimeUtc();
    }
    m_records.prepend(std::move(record));
    prune();
    if (save()) {
        emit changed();
    }
}

void HistoryStore::clear() {
    m_records.clear();
    m_loaded = true;
    QFile file(m_path);
    if (file.exists() && !file.remove()) {
        emit storeError(QStringLiteral("Could not remove the local history file."));
        return;
    }
    emit changed();
}

const QVector<TranslationRecord>& HistoryStore::records() const {
    return m_records;
}

QString HistoryStore::storagePath() const {
    return m_path;
}

void HistoryStore::load() {
    m_loaded = true;
    m_records.clear();

    const QFileInfo info(m_path);
    if (!info.exists()) {
        return;
    }
    if (info.isSymLink() || !info.isFile() || info.size() > kMaximumHistoryBytes) {
        emit storeError(QStringLiteral("The local history file is unsafe or unexpectedly large."));
        return;
    }

    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly)) {
        emit storeError(QStringLiteral("Could not read the local history file."));
        return;
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isArray()) {
        emit storeError(QStringLiteral("The local history file is damaged."));
        return;
    }

    for (const QJsonValue& value : document.array()) {
        const QJsonObject object = value.toObject();
        TranslationRecord record;
        record.id = object.value(QStringLiteral("id")).toString();
        record.createdAt = QDateTime::fromString(object.value(QStringLiteral("createdAt")).toString(),
                                                 Qt::ISODateWithMs);
        record.sourceCode = object.value(QStringLiteral("source")).toString();
        record.targetCode = object.value(QStringLiteral("target")).toString();
        record.sourceText = object.value(QStringLiteral("input")).toString();
        record.translatedText = object.value(QStringLiteral("output")).toString();
        record.model = object.value(QStringLiteral("model")).toString();
        if (!record.id.isEmpty() && record.createdAt.isValid()) {
            m_records.push_back(std::move(record));
        }
    }
    std::sort(m_records.begin(), m_records.end(),
              [](const TranslationRecord& left, const TranslationRecord& right) {
                  return left.createdAt > right.createdAt;
              });
    prune();
}

void HistoryStore::prune() {
    const QDateTime cutoff = QDateTime::currentDateTimeUtc().addDays(-m_retentionDays);
    m_records.erase(std::remove_if(m_records.begin(), m_records.end(),
                                   [&](const TranslationRecord& record) {
                                       return record.createdAt < cutoff;
                                   }),
                    m_records.end());
    if (m_records.size() > m_maximumRecords) {
        m_records.resize(m_maximumRecords);
    }
}

bool HistoryStore::save() {
    const QFileInfo existing(m_path);
    if (existing.exists() && existing.isSymLink()) {
        emit storeError(QStringLiteral("Refusing to write history through a symbolic link."));
        return false;
    }

    QDir directory = existing.dir();
    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
        emit storeError(QStringLiteral("Could not create the private history directory."));
        return false;
    }

    QJsonArray array;
    for (const TranslationRecord& record : m_records) {
        array.append(QJsonObject{{QStringLiteral("id"), record.id},
                                 {QStringLiteral("createdAt"),
                                  record.createdAt.toUTC().toString(Qt::ISODateWithMs)},
                                 {QStringLiteral("source"), record.sourceCode},
                                 {QStringLiteral("target"), record.targetCode},
                                 {QStringLiteral("input"), record.sourceText},
                                 {QStringLiteral("output"), record.translatedText},
                                 {QStringLiteral("model"), record.model}});
    }

    QSaveFile file(m_path);
    if (!file.open(QIODevice::WriteOnly)) {
        emit storeError(QStringLiteral("Could not open the private history file for writing."));
        return false;
    }
    const QByteArray payload = QJsonDocument(array).toJson(QJsonDocument::Compact);
    if (file.write(payload) != payload.size() || !file.commit()) {
        emit storeError(QStringLiteral("Could not commit the local history file atomically."));
        return false;
    }
    QFile::setPermissions(m_path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    return true;
}

} // namespace translunix
