#include "spacesearchbox.h"

SpaceSearchBox::SpaceSearchBox(QWidget *parent) : QLineEdit(parent)
{

}

void SpaceSearchBox::focusInEvent(QFocusEvent *event) {
    event->accept();
    updateText();
    this->selectAll();
    emit GotFocus();
}

void SpaceSearchBox::focusOutEvent(QFocusEvent *event) {
    event->accept();
    updateText();
}

void SpaceSearchBox::setCurrentUrl(QUrl currentUrl) {
    url = currentUrl;

    updateText();
}

QUrl SpaceSearchBox::currentUrl() {
    return url;
}

void SpaceSearchBox::setCurrentSecurity(SecurityType securityType) {
    this->sec = securityType;
}

void SpaceSearchBox::updateText() {
    if (currentUrl().host().startsWith("www.google.")) {
        QString query;
        QStringList params = currentUrl().query().split("&");
        for (QString p : params) {
            if (p.startsWith("q")) {
                query = p.split("=").at(1);
            }
        }
        if (query == "") {
            params = currentUrl().fragment().split("&");
            for (QString p : params) {
                if (p.startsWith("q")) {
                    query = p.split("=").at(1);
                }
            }
            if (query == "") {
                if (this->hasFocus()) {
                    this->setText(currentUrl().toString());
                } else {
                    this->setText(currentUrl().host());
                }
            } else {
                this->setText(query.replace("+", " "));
            }
        } else {
            this->setText(query.replace("+", " "));
        }
    } else {
        if (this->hasFocus()) {
            this->setText(currentUrl().toString());
        } else {
            this->setText(currentUrl().host());
        }
    }
}

SpaceSearchBox::SecurityType SpaceSearchBox::CurrentSecurity() {
    return sec;
}
