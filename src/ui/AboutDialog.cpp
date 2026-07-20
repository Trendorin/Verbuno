#include "ui/AboutDialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

namespace verbuno {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("About Verbuno"));
    setModal(true);
    setMinimumWidth(440);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(12);

    auto* icon = new QLabel(this);
    icon->setAlignment(Qt::AlignCenter);
    icon->setPixmap(QPixmap(QStringLiteral(":/icons/app.svg"))
                        .scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(icon);

    auto* name = new QLabel(QStringLiteral("Verbuno %1").arg(QStringLiteral(VERBUNO_VERSION)),
                            this);
    QFont nameFont = name->font();
    nameFont.setBold(true);
    nameFont.setPointSize(nameFont.pointSize() + 3);
    name->setFont(nameFont);
    name->setAlignment(Qt::AlignCenter);
    layout->addWidget(name);

    auto* description = new QLabel(
        tr("A native C++20 and Qt 6 Widgets translation client for Linux with local photo "
           "OCR.\n\nVerbuno contains no telemetry. Photos are recognized on this device. Only text "
           "you explicitly translate is sent to the API endpoint you configure. OpenRouter "
           "and the selected upstream provider apply their own logging, retention, and "
           "training policies."),
        this);
    description->setWordWrap(true);
    description->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(description);

    auto* links = new QLabel(
        QStringLiteral("<a href=\"https://github.com/Trendorin/Verbuno\">GitHub</a> · "
                       "<a href=\"https://github.com/Trendorin/Verbuno/blob/main/docs/PRIVACY.md\">"
                       "Privacy</a> · GPL-3.0-or-later"),
        this);
    links->setOpenExternalLinks(true);
    links->setAlignment(Qt::AlignCenter);
    layout->addWidget(links);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

} // namespace verbuno
