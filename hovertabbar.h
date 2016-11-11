#ifndef HOVERTABBAR_H
#define HOVERTABBAR_H

#include <QTabBar>
#include <QMouseEvent>
#include <QSettings>
#include <QMimeData>
#include "mainwindow.h"

class MainWindow;

class HoverTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit HoverTabBar(MainWindow *parent = 0);

signals:
    void previewTab(int indexToPreview);
    void cancelPreview();

public slots:
    void setStyleSheet(QString styleSheet = "");

private:
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent* event);

    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dropEvent(QDropEvent* event);

    int currentHoverTab;
    QSettings settings;
    QString currentStyleSheet;

    MainWindow* mainWindow;
};

#endif // HOVERTABBAR_H
