#include "animatedstackedwidget.h"

AnimatedStackedWidget::AnimatedStackedWidget(QWidget *parent) : QStackedWidget(parent)
{

}

void AnimatedStackedWidget::setCurrentIndex(int index, bool isNew) {
    QWidget* currentWidget = widget(currentIndex());
    if (currentIndex() != index && !doingNewAnimation) {
        QWidget* nextWidget = widget(index);
        if (nextWidget == NULL) {
            QStackedWidget::setCurrentIndex(index);
        } else {
            if (isNew) {
                if (settings.value("browser/toolbarOnBottom").toBool()) {
                    nextWidget->setGeometry(0, -this->height(), this->width(), this->height());
                } else {
                    nextWidget->setGeometry(0, this->height(), this->width(), this->height());
                }
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
                if (widgetToRemove != NULL) {
                    QStackedWidget::removeWidget(widgetToRemove);
                    widgetToRemove == NULL;
                }
            });
            group->start();
        }
    }
}

void AnimatedStackedWidget::removeWidget(QWidget *w) {
    int index = indexOf(w);
    if (index == currentIndex()) {
        widgetToRemove = w;
        if (index == count() - 1) {
            setCurrentIndex(count() - 2);
        } else {
            setCurrentIndex(index + 1);
        }
    } else {
        QStackedWidget::removeWidget(w);
    }
}
