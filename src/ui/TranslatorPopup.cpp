#include "ui/TranslatorPopup.h"

#include "ui/TranslationPanel.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QStyle>
#include <QVBoxLayout>

namespace translunix {

TranslatorPopup::TranslatorPopup(TranslationController* controller,
                                 AppSettings* settings,
                                 QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) {
    setObjectName(QStringLiteral("translatorPopup"));
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral(
        "#translatorPopup { background: palette(window); border: 1px solid palette(mid); }"));
    setMinimumSize(520, 570);
    resize(540, 610);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* header = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 8, 8, 6);
    auto* icon = new QLabel(header);
    icon->setPixmap(QIcon(QStringLiteral(":/icons/app.svg")).pixmap(24, 24));
    auto* title = new QLabel(QStringLiteral("TranslUnix"), header);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    auto* expand = new QPushButton(tr("Open"), header);
    expand->setToolTip(tr("Open the full window"));
    auto* settingsButton = new QPushButton(tr("Settings"), header);
    auto* closeButton = new QPushButton(header);
    closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    closeButton->setToolTip(tr("Hide"));
    closeButton->setFlat(true);
    headerLayout->addWidget(icon);
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(expand);
    headerLayout->addWidget(settingsButton);
    headerLayout->addWidget(closeButton);
    root->addWidget(header);

    m_panel = new TranslationPanel(controller, settings, true, this);
    root->addWidget(m_panel, 1);

    m_geometryAnimation = new QPropertyAnimation(this, "geometry", this);
    m_geometryAnimation->setDuration(170);
    m_geometryAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_opacityAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_opacityAnimation->setDuration(140);
    m_opacityAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(closeButton, &QPushButton::clicked, this, &QWidget::hide);
    connect(expand, &QPushButton::clicked, this, [this] {
        hide();
        emit fullWindowRequested();
    });
    connect(settingsButton, &QPushButton::clicked, this, [this] {
        hide();
        emit settingsRequested();
    });
}

void TranslatorPopup::showAnimated(const QRect& anchor) {
    m_geometryAnimation->stop();
    m_opacityAnimation->stop();

    const QRect end = targetGeometry(anchor);
    QRect start = end;
    start.translate(0, 18);
    setGeometry(start);
    setWindowOpacity(0.0);
    show();
    raise();
    activateWindow();

    m_geometryAnimation->setStartValue(start);
    m_geometryAnimation->setEndValue(end);
    m_opacityAnimation->setStartValue(0.0);
    m_opacityAnimation->setEndValue(1.0);
    m_geometryAnimation->start();
    m_opacityAnimation->start();
    focusInput();
}

void TranslatorPopup::toggle(const QRect& anchor) {
    if (isVisible()) {
        hide();
    } else {
        showAnimated(anchor);
    }
}

void TranslatorPopup::focusInput() {
    m_panel->focusInput();
}

bool TranslatorPopup::event(QEvent* event) {
    if (event->type() == QEvent::WindowDeactivate && isVisible() &&
        QApplication::activeModalWidget() == nullptr) {
        hide();
    }
    return QWidget::event(event);
}

QRect TranslatorPopup::targetGeometry(const QRect& anchor) const {
    QScreen* screen = nullptr;
    if (anchor.isValid()) {
        screen = QGuiApplication::screenAt(anchor.center());
    }
    if (!screen) {
        screen = QGuiApplication::screenAt(QCursor::pos());
    }
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect available = screen ? screen->availableGeometry() : QRect(0, 0, 1280, 720);
    const QSize popupSize = size().boundedTo(available.size() - QSize(20, 20));
    int x = available.right() - popupSize.width() - 12;
    int y = available.bottom() - popupSize.height() - 12;
    if (anchor.isValid()) {
        x = anchor.center().x() - popupSize.width() / 2;
        y = anchor.top() - popupSize.height() - 8;
        if (y < available.top() + 8) {
            y = anchor.bottom() + 8;
        }
    }
    x = qBound(available.left() + 8, x, available.right() - popupSize.width() - 8);
    y = qBound(available.top() + 8, y, available.bottom() - popupSize.height() - 8);
    return {QPoint(x, y), popupSize};
}

} // namespace translunix
