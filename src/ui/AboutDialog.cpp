#include "ui/AboutDialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

namespace translunix {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("About TranslUnix"));
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

    auto* name = new QLabel(QStringLiteral("TranslUnix %1").arg(QStringLiteral(TRANSLUNIX_VERSION)),
                            this);
    QFont nameFont = name->font();
    nameFont.setBold(true);
    nameFont.setPointSize(nameFont.pointSize() + 3);
    name->setFont(nameFont);
    name->setAlignment(Qt::AlignCenter);
    layout->addWidget(name);

    auto* description = new QLabel(
        tr("A native C++20 and Qt 6 Widgets translation client for Linux.\n\n"
           "TranslUnix contains no telemetry. Translation text is sent only to the API endpoint "
           "you configure. The selected provider and model remain responsible for upstream "
           "logging, retention, and training policies."),
        this);
    description->setWordWrap(true);
    description->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(description);

    auto* links = new QLabel(
        QStringLiteral("<a href=\"https://github.com/Trendorin/TranslUnix\">GitHub</a> · "
                       "<a href=\"https://github.com/Trendorin/TranslUnix/blob/main/docs/PRIVACY.md\">"
                       "Privacy</a> · GPL-3.0-or-later"),
        this);
    links->setOpenExternalLinks(true);
    links->setAlignment(Qt::AlignCenter);
    layout->addWidget(links);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

} // namespace translunix
