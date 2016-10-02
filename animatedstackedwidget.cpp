#include "animatedstackedwidget.h"

AnimatedStackedWidget::AnimatedStackedWidget(QWidget *parent) : QStackedWidget(parent)
{

}

void AnimatedStackedWidget::setCurrentIndex(int index, bool isNew) {
    if (currentIndex() != index && !doingNewAnimation) {
    doSetCurrentIndex(index, isNew);
    }
}

void AnimatedStackedWidget::doSetCurrentIndex(int index, bool isNew) {
    QWidget* currentWidget = widget(currentIndex());
    QWidget* nextWidget = widget(index);
    if (nextWidget == NULL) {
        QStackedWidget::setCurrentIndex(index);
    } else {
        if (currentPreview == index || pendingPreview == index) {
            if (doingPreviewAnimation) {
                currentPreviewAnimation->stop();
                emit currentPreviewAnimation->finished();
            }

            currentPreview = -1;
            pendingPreview = -1;
            currentPreviewWidget = NULL;
            currentPreviewAnimation = NULL;
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
        }
        nextWidget->repaint();
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
                widgetToRemove = NULL;
            }
        });
        group->start();
    }
}

void AnimatedStackedWidget::removeWidget(QWidget *w) {
    int index = indexOf(w);
    if (index == currentPreview) {
        currentPreview = -1;
        pendingPreview = -1;
        currentPreviewWidget = NULL;
        currentPreviewAnimation = NULL;
    }
    if (currentPreviewAnimation != NULL) {
        currentPreviewAnimation->stop();
        emit currentPreviewAnimation->finished();
    }
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

void AnimatedStackedWidget::previewTab(int tabToPreview) {
    if ((currentPreview != tabToPreview && pendingPreview != tabToPreview) && currentPreview != currentIndex() && !doingNewAnimation) {
        if (currentPreviewAnimation != NULL) {
            currentPreviewAnimation->stop();
            emit currentPreviewAnimation->finished();
        }

        doingPreviewAnimation = true;
        QWidget* newWidget = widget(tabToPreview);
        if (newWidget != NULL) {
            newWidget->show();
            newWidget->raise();

            QSequentialAnimationGroup* animationGroup = new QSequentialAnimationGroup;

            QPropertyAnimation* newWidgetAnim = new QPropertyAnimation(newWidget, "geometry");
            if (currentPreview != -1) {
                QPropertyAnimation* oldTabAnim = new QPropertyAnimation(currentPreviewWidget, "geometry");
                oldTabAnim->setStartValue(currentPreviewWidget->geometry());
                oldTabAnim->setEndValue(QRect(this->width(), 0, this->width(), this->height()));
                oldTabAnim->setEasingCurve(QEasingCurve::OutCubic);
                oldTabAnim->setDuration(250);
                animationGroup->addAnimation(oldTabAnim);
                currentPreviewWidget->raise();

                //newWidgetAnim->setStartValue(QRect(50, this->height(), this->width(), this->height()));
            }
            newWidgetAnim->setStartValue(QRect(this->width(), 0, this->width(), this->height()));
            newWidgetAnim->setEndValue(QRect(50, 0, this->width(), this->height()));
            newWidgetAnim->setEasingCurve(QEasingCurve::OutCubic);
            newWidgetAnim->setDuration(250);
            animationGroup->addAnimation(newWidgetAnim);

            pendingPreview = tabToPreview;

            connect(animationGroup, &QSequentialAnimationGroup::finished, [=]() {
                if (currentPreview != -1) {
                    currentPreviewWidget->hide();
                }
                currentPreview = tabToPreview;
                currentPreviewWidget = newWidget;
                pendingPreview = -1;
                doingPreviewAnimation = false;
                currentPreviewAnimation = NULL;
            });
            connect(animationGroup, SIGNAL(finished()), animationGroup, SLOT(deleteLater()));
            animationGroup->start();
            currentPreviewAnimation = animationGroup;
        }
    }
}

void AnimatedStackedWidget::cancelPreview() {
    if (currentPreview == currentIndex()) {
        doSetCurrentIndex(currentIndex(), false);
    } else if ((currentPreview != -1 || pendingPreview != -1) && !doingNewAnimation) {
        if (doingPreviewAnimation) {
            currentPreviewAnimation->stop();
            emit currentPreviewAnimation->finished();
            if (currentPreview == -1 && pendingPreview == -1) {
                return;
            }
        }
        doingPreviewAnimation = true;
        currentPreviewWidget->raise();
        QPropertyAnimation* oldTabAnim = new QPropertyAnimation(currentPreviewWidget, "geometry");
        oldTabAnim->setStartValue(currentPreviewWidget->geometry());
        oldTabAnim->setEndValue(QRect(this->width(), 0, this->width(), this->height()));
        oldTabAnim->setEasingCurve(QEasingCurve::OutCubic);
        oldTabAnim->setDuration(250);

        connect(oldTabAnim, &QParallelAnimationGroup::finished, [=]() {
            currentPreviewWidget->hide();
            currentPreview = -1;
            pendingPreview = -1;
            currentPreviewWidget = NULL;
            doingPreviewAnimation = false;
            currentPreviewAnimation = NULL;
        });
        connect(oldTabAnim, SIGNAL(finished()), oldTabAnim, SLOT(deleteLater()));
        oldTabAnim->start();
        currentPreviewAnimation = oldTabAnim;
    }
}
