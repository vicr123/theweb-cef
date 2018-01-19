#ifndef TABBUTTON_H
#define TABBUTTON_H

#include <QPushButton>
#include <QPalette>
#include <QPainter>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <functional>

class TabButton : public QPushButton
{
    Q_OBJECT

    public:
        explicit TabButton(QWidget* parent = nullptr);

    public slots:
        void setLoading(bool isLoading);
        void setText(QString text);
        void setSiteIcon(QStringList urls);

    signals:
        void textChange(QString newText);

    private:
        void paintEvent(QPaintEvent* event);
        void enterEvent(QEvent* event);
        void leaveEvent(QEvent* event);

        void downloadUrl(QStringList urls, int url);

        bool isLoading = false;
        bool hovering = false;
        uint animationCounter;
        QTimer* animationTimer;
        QNetworkAccessManager mgr;
};

#endif // TABBUTTON_H
