#include "hovertabbar.h"

HoverTabBar::HoverTabBar(QWidget *parent) : QTabBar(parent)
{
    this->setMouseTracking(true);
}

void HoverTabBar::mouseMoveEvent(QMouseEvent *event) {
    int tabAtIndex = tabAt(event->pos());
    if (tabAtIndex != this->currentIndex() && tabAtIndex != -1) {
        emit previewTab(tabAtIndex);
    } else if (tabAtIndex == this->currentIndex()) {
        emit this->cancelPreview();
    }
}

void HoverTabBar::leaveEvent(QEvent *event) {
    emit this->cancelPreview();
}

