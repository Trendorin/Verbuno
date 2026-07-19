#include "ui/HistoryPage.h"

#include "core/TranslationController.h"

#include <QDateTime>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace translunix {

HistoryPage::HistoryPage(TranslationController* controller, QWidget* parent)
    : QWidget(parent)
    , m_controller(controller) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    auto* header = new QHBoxLayout;
    auto* title = new QLabel(tr("Local history"), this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);
    title->setFont(titleFont);
    auto* clearButton = new QPushButton(tr("Clear history"), this);
    header->addWidget(title);
    header->addStretch();
    header->addWidget(clearButton);
    root->addLayout(header);

    m_emptyLabel = new QLabel(
        tr("History is empty. It is disabled by default and can be enabled in Privacy settings."),
        this);
    m_emptyLabel->setWordWrap(true);
    root->addWidget(m_emptyLabel);

    auto* splitter = new QSplitter(Qt::Vertical, this);
    m_table = new QTableWidget(splitter);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels(
        {tr("Time"), tr("Languages"), tr("Text"), tr("Model")});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    auto* details = new QWidget(splitter);
    auto* detailsLayout = new QHBoxLayout(details);
    detailsLayout->setContentsMargins(0, 8, 0, 0);
    m_source = new QPlainTextEdit(details);
    m_source->setReadOnly(true);
    m_source->setPlaceholderText(tr("Source text"));
    m_translation = new QPlainTextEdit(details);
    m_translation->setReadOnly(true);
    m_translation->setPlaceholderText(tr("Translation"));
    detailsLayout->addWidget(m_source);
    detailsLayout->addWidget(m_translation);
    splitter->addWidget(m_table);
    splitter->addWidget(details);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    root->addWidget(splitter, 1);

    connect(m_controller, &TranslationController::historyChanged, this, &HistoryPage::reload);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &HistoryPage::showSelection);
    connect(clearButton, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, tr("Clear local history"),
                                  tr("Delete every locally stored translation?")) ==
            QMessageBox::Yes) {
            m_controller->clearHistory();
        }
    });
    reload();
}

void HistoryPage::reload() {
    const QVector<TranslationRecord>& records = m_controller->historyRecords();
    m_table->setRowCount(records.size());
    for (qsizetype row = 0; row < records.size(); ++row) {
        const TranslationRecord& record = records.at(row);
        const QString preview = record.sourceText.simplified().left(100);
        m_table->setItem(static_cast<int>(row), 0,
                         new QTableWidgetItem(record.createdAt.toLocalTime().toString(
                             QStringLiteral("yyyy-MM-dd HH:mm"))));
        m_table->setItem(static_cast<int>(row), 1,
                         new QTableWidgetItem(
                             QStringLiteral("%1 → %2").arg(record.sourceCode, record.targetCode)));
        m_table->setItem(static_cast<int>(row), 2, new QTableWidgetItem(preview));
        m_table->setItem(static_cast<int>(row), 3, new QTableWidgetItem(record.model));
    }
    m_emptyLabel->setVisible(records.isEmpty());
    m_table->setVisible(!records.isEmpty());
    if (records.isEmpty()) {
        m_source->clear();
        m_translation->clear();
    } else {
        m_table->selectRow(0);
    }
}

void HistoryPage::showSelection() {
    const int row = m_table->currentRow();
    const QVector<TranslationRecord>& records = m_controller->historyRecords();
    if (row < 0 || row >= records.size()) {
        return;
    }
    const TranslationRecord& record = records.at(row);
    m_source->setPlainText(record.sourceText);
    m_translation->setPlainText(record.translatedText);
}

} // namespace translunix
