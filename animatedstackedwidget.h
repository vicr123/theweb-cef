#ifndef ANIMATEDSTACKEDWIDGET_H
#define ANIMATEDSTACKEDWIDGET_H

#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QSettings>
#include <QWindow>
#include <QDebug>
#include <QApplication>

class AnimatedStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit AnimatedStackedWidget(QWidget *parent = 0);

    void removeWidget(QWidget *w);
signals:

public slots:
    void setCurrentIndex(int index, bool isNew = false);
    void previewTab(int tabToPreview);
    void cancelPreview();

private:
    void doSetCurrentIndex(int index, bool isNew);
    bool doingNewAnimation = false;
    bool doingPreviewAnimation = false;
    QSettings settings;
    QWidget* widgetToRemove = NULL;

    QAbstractAnimation* currentPreviewAnimation = NULL;
    int currentPreview = -1;
    int pendingPreview = -1;
    QWidget* currentPreviewWidget = NULL;
    bool pausePreviewAnimation = false;
};

#endif // ANIMATEDSTACKEDWIDGET_H
