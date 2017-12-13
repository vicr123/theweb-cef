#ifndef BAR_H
#define BAR_H

#include <QLineEdit>
#include <QUrl>

class Bar : public QLineEdit
{
        Q_OBJECT
    public:
        explicit Bar(QWidget *parent = nullptr);

    signals:

    public slots:
        void setUrl(QUrl url);

    private:
        QUrl url;
        bool needsHighlight = false;
        void updateText();

        void focusInEvent(QFocusEvent* event);
        void focusOutEvent(QFocusEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
};

#endif // BAR_H
