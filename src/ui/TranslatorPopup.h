#pragma once

#include <QRect>
#include <QWidget>

class QPropertyAnimation;

namespace translunix {

class AppSettings;
class TranslationController;
class TranslationPanel;

class TranslatorPopup final : public QWidget {
    Q_OBJECT

public:
    TranslatorPopup(TranslationController* controller,
                    AppSettings* settings,
                    QWidget* parent = nullptr);

    void showAnimated(const QRect& anchor = {});
    void toggle(const QRect& anchor = {});
    void focusInput();

signals:
    void settingsRequested();
    void fullWindowRequested();

protected:
    bool event(QEvent* event) override;

private:
    [[nodiscard]] QRect targetGeometry(const QRect& anchor) const;

    TranslationPanel* m_panel = nullptr;
    QPropertyAnimation* m_geometryAnimation = nullptr;
    QPropertyAnimation* m_opacityAnimation = nullptr;
};

} // namespace translunix
