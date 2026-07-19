#pragma once

#include "core/TranslationTypes.h"

#include <QObject>
#include <QString>
#include <QVector>

namespace translunix {

class HistoryStore final : public QObject {
    Q_OBJECT

public:
    explicit HistoryStore(QString storagePath = {}, QObject* parent = nullptr);

    void setEnabled(bool enabled);
    [[nodiscard]] bool isEnabled() const;

    void setPolicy(int maximumRecords, int retentionDays);
    void append(TranslationRecord record);
    void clear();

    [[nodiscard]] const QVector<TranslationRecord>& records() const;
    [[nodiscard]] QString storagePath() const;

signals:
    void changed();
    void storeError(const QString& message);

private:
    void load();
    void prune();
    [[nodiscard]] bool save();

    QString m_path;
    QVector<TranslationRecord> m_records;
    bool m_enabled = false;
    bool m_loaded = false;
    int m_maximumRecords = 100;
    int m_retentionDays = 30;
};

} // namespace translunix
