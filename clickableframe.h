#ifndef CLICKABLEFRAME_H
#define CLICKABLEFRAME_H

#include <QFrame>
#include <QMouseEvent>

class ClickableFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ClickableFrame(QWidget *parent = 0);

signals:
    void clicked();

public slots:

private:
    void mouseReleaseEvent(QMouseEvent* event);
};

#endif // CLICKABLEFRAME_H
