#include "tabbutton.h"

TabButton::TabButton(QWidget* parent) : QPushButton(parent)
{
    animationTimer = new QTimer();
    animationTimer->setInterval(10);
    connect(animationTimer, &QTimer::timeout, [=] {
        animationCounter += (this->width() / 50) + 1;
        this->update();
    });
}

void TabButton::setLoading(bool isLoading) {
    this->isLoading = isLoading;
    this->update();

    if (isLoading) {
        animationTimer->start();
    } else {
        animationTimer->stop();
    }
}

void TabButton::paintEvent(QPaintEvent *event) {
    QPushButton::paintEvent(event);

    if (isLoading) {
        QRect rect;
        rect.setTop(this->height() - 4);
        rect.setLeft(-this->width() + animationCounter % (this->width() * 2));
        rect.setHeight(4);
        rect.setWidth(this->width());

        QPainter painter(this);
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(0, 150, 255));
        painter.drawRect(rect);
    }
}

void TabButton::setText(QString text) {
    QPushButton::setText(text);
    emit textChange(text);
}
