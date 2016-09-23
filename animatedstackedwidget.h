#ifndef ANIMATEDSTACKEDWIDGET_H
#define ANIMATEDSTACKEDWIDGET_H

#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>

class AnimatedStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit AnimatedStackedWidget(QWidget *parent = 0);

signals:

public slots:
    void setCurrentIndex(int index, bool isNew = false);

private:
    bool doingNewAnimation = false;
};

#endif // ANIMATEDSTACKEDWIDGET_H
