#include "spacesearchbox.h"

SpaceSearchBox::SpaceSearchBox(QWidget *parent) : QLineEdit(parent)
{

}

void SpaceSearchBox::focusInEvent(QFocusEvent *event) {
    event->accept();
    this->setText(url.toString());
    this->selectAll();
    emit GotFocus();
}

void SpaceSearchBox::focusOutEvent(QFocusEvent *event) {
    event->accept();
    this->setText(url.host());
}

void SpaceSearchBox::setCurrentUrl(QUrl currentUrl) {
    url = currentUrl;

    if (!this->hasFocus()) {
        this->setText(currentUrl.host());
    }
}

QUrl SpaceSearchBox::currentUrl() {
    return url;
}

void SpaceSearchBox::setCurrentSecurity(SecurityType securityType) {
    this->sec = securityType;
}

SpaceSearchBox::SecurityType SpaceSearchBox::CurrentSecurity() {
    return sec;
}
