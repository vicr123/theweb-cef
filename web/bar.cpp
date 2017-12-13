#include "bar.h"

Bar::Bar(QWidget *parent) : QLineEdit(parent)
{
    this->setCursor(QCursor(Qt::ArrowCursor));
}

void Bar::setUrl(QUrl url) {
    this->url = url;
    updateText();
}

void Bar::focusInEvent(QFocusEvent *event) {
    this->setCursor(QCursor(Qt::IBeamCursor));
    this->setText(url.toEncoded());
    needsHighlight = true;
}

void Bar::focusOutEvent(QFocusEvent *event) {
    this->setCursor(QCursor(Qt::ArrowCursor));
    updateText();
}

void Bar::mouseReleaseEvent(QMouseEvent *event) {
    if (needsHighlight) {
        this->selectAll();
        needsHighlight = false;
    }
}

void Bar::updateText() {
    if (!this->hasFocus()) {
        this->setText(url.host());
    }
}
