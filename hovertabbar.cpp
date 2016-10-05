#include "hovertabbar.h"

HoverTabBar::HoverTabBar(QWidget *parent) : QTabBar(parent)
{
    this->setMouseTracking(true);
}

void HoverTabBar::mouseMoveEvent(QMouseEvent *event) {
    if (settings.value("behaviour/tabPreview", false).toBool()) {
        int tabAtIndex = tabAt(event->pos());
        if (currentHoverTab != tabAtIndex) {
            if (tabAtIndex != this->currentIndex() && tabAtIndex != -1) {
                emit previewTab(tabAtIndex);
            } else if (tabAtIndex == this->currentIndex()) {
                emit this->cancelPreview();
            }
            currentHoverTab = tabAtIndex;
        }
    }
    event->accept();
}

void HoverTabBar::leaveEvent(QEvent *event) {
    //Don't check the setting here (just in case)
    emit this->cancelPreview();
    currentHoverTab = -1;
}
