#include "animatedstackedwidget.h"

AnimatedStackedWidget::AnimatedStackedWidget(QWidget *parent) : QStackedWidget(parent)
{

}

void AnimatedStackedWidget::setCurrentIndex(int index, bool isNew) {
    QWidget* currentWidget = widget(currentIndex());
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

        QSequentialAnimationGroup* group = new QSequentialAnimationGroup;

        QPropertyAnimation* animation = new QPropertyAnimation(nextWidget, "geometry");
        animation->setStartValue(nextWidget->geometry());
        animation->setEndValue(QRect(0, 0, this->width(), this->height()));
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->setDuration(250);
        group->addAnimation(animation);

        if (isNew) {
            QPropertyAnimation* oldAnimation = new QPropertyAnimation(currentWidget, "geometry");
            oldAnimation->setStartValue(currentWidget->geometry());
            oldAnimation->setEndValue(QRect(-this->width(), 0, this->width(), this->height()));
            oldAnimation->setEasingCurve(QEasingCurve::OutCubic);
            oldAnimation->setDuration(250);
            group->insertAnimation(0, oldAnimation);
        }

        connect(group, &QSequentialAnimationGroup::finished, [=]() {
            QStackedWidget::setCurrentIndex(index);
            doingNewAnimation = false;
        });
        group->start();
    }
}
