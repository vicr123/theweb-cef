#ifndef TABBUTTON_H
#define TABBUTTON_H

#include <QPushButton>
#include <QPalette>
#include <QPainter>
#include <QTimer>

class TabButton : public QPushButton
{
    Q_OBJECT

    public:
        explicit TabButton(QWidget* parent = nullptr);

    public slots:
        void setLoading(bool isLoading);

    private:
        void paintEvent(QPaintEvent* event);
        bool isLoading = false;
        uint animationCounter;
        QTimer* animationTimer;
};

#endif // TABBUTTON_H
