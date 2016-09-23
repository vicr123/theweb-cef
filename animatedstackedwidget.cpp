#include "animatedstackedwidget.h"

AnimatedStackedWidget::AnimatedStackedWidget(QWidget *parent) : QStackedWidget(parent)
{

}

void AnimatedStackedWidget::setCurrentIndex(int index, bool isNew) {
    //QWidget* currentWidget = widget(currentIndex());
    if (currentIndex() != index && !doingNewAnimation) {
        QWidget* nextWidget = widget(index);

        if (isNew) {
            nextWidget->setGeometry(0, this->height(), this->width(), this->height());
            doingNewAnimation = true;
        } else {
            if (currentIndex() < index) {
                nextWidget->setGeometry(this->width(), 0, this->width(), this->height());
            } else {
                nextWidget->setGeometry(-this->width(), 0, this->width(), this->height());
            }
        }
        nextWidget->show();
        nextWidget->raise();

        QPropertyAnimation* animation = new QPropertyAnimation(nextWidget, "geometry");
        animation->setStartValue(nextWidget->geometry());
        animation->setEndValue(QRect(0, 0, this->width(), this->height()));
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->setDuration(250);
        connect(animation, &QPropertyAnimation::finished, [=]() {
            QStackedWidget::setCurrentIndex(index);
            doingNewAnimation = false;
        });
        animation->start();
    }
}
