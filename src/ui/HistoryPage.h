#pragma once

#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QTableWidget;

namespace translunix {

class TranslationController;

class HistoryPage final : public QWidget {
    Q_OBJECT

public:
    explicit HistoryPage(TranslationController* controller, QWidget* parent = nullptr);

private:
    void reload();
    void showSelection();

    TranslationController* m_controller;
    QTableWidget* m_table = nullptr;
    QPlainTextEdit* m_source = nullptr;
    QPlainTextEdit* m_translation = nullptr;
    QLabel* m_emptyLabel = nullptr;
};

} // namespace translunix
