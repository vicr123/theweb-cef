#include "clickableframe.h"

ClickableFrame::ClickableFrame(QWidget *parent) : QFrame(parent)
{

}

void ClickableFrame::mouseReleaseEvent(QMouseEvent *event) {
    emit clicked();
}
