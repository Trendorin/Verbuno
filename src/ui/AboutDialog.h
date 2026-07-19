#pragma once

#include <QDialog>

namespace verbuno {

class AboutDialog final : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
};

} // namespace verbuno
