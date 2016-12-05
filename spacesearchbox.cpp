#include "spacesearchbox.h"

SpaceSearchBox::SpaceSearchBox(QWidget *parent) : QLineEdit(parent)
{

}

void SpaceSearchBox::focusInEvent(QFocusEvent *event) {
    event->accept();
    updateText();
    this->selectAll();
    this->setCursor(QCursor(Qt::IBeamCursor));
    emit GotFocus();
}

void SpaceSearchBox::focusOutEvent(QFocusEvent *event) {
    event->accept();
    updateText();
    this->setCursor(QCursor(Qt::ArrowCursor));
}

void SpaceSearchBox::setCurrentUrl(QUrl currentUrl) {
    url = currentUrl;

    if (!this->hasFocus()) {
        updateText();
    }
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

void SpaceSearchBox::setHoverText(QString text) {
    hoverText = text;
    this->update();
}

void SpaceSearchBox::clearHoverText() {
    hoverText = "";
    this->update();
}

void SpaceSearchBox::paintEvent(QPaintEvent *event) {
    QLineEdit::paintEvent(event);

    if (hoverText != "") {
        int width = this->width() - 30;
        width -= this->fontMetrics().width(this->text());

        QString renderText = this->fontMetrics().elidedText(hoverText, Qt::ElideMiddle, width);

        QPainter painter(this);
        painter.setPen(this->palette().color(QPalette::Disabled, QPalette::WindowText));
        painter.drawText(0, 0, this->width() - 5, this->height(), Qt::AlignVCenter | Qt::AlignRight, renderText);
    }
}
