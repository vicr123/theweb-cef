#ifndef ANIMATEDSTACKEDWIDGET_H
#define ANIMATEDSTACKEDWIDGET_H

#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QSettings>
#include <QWindow>

class AnimatedStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit AnimatedStackedWidget(QWidget *parent = 0);

    void removeWidget(QWidget *w);
signals:

public slots:
    void setCurrentIndex(int index, bool isNew = false);

private:
    bool doingNewAnimation = false;
    QSettings settings;
    QWidget* widgetToRemove = NULL;
};

#endif // ANIMATEDSTACKEDWIDGET_H
