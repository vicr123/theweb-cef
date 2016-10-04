#ifndef HOVERTABBAR_H
#define HOVERTABBAR_H

#include <QTabBar>
#include <QMouseEvent>
#include <QSettings>

class HoverTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit HoverTabBar(QWidget *parent = 0);

signals:
    void previewTab(int indexToPreview);
    void cancelPreview();

public slots:

private:
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent* event);

    int currentHoverTab;
    QSettings settings;
};

#endif // HOVERTABBAR_H
