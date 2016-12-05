#ifndef SPACESEARCHBOX_H
#define SPACESEARCHBOX_H

#include <QLineEdit>
#include <QFocusEvent>
#include <QUrl>
#include <QPaintEvent>
#include <QDebug>
#include <QPainter>
#include <QFontMetrics>

class SpaceSearchBox : public QLineEdit
{
    Q_OBJECT

    enum SecurityType {
        None,
        Secure,
        SuperSecure,
        SecureError
    };

    Q_PROPERTY(QUrl currentUrl READ currentUrl WRITE setCurrentUrl)
    Q_PROPERTY(SecurityType CurrentSecurity READ CurrentSecurity WRITE setCurrentSecurity)

public:
    explicit SpaceSearchBox(QWidget *parent = 0);

    void setCurrentUrl(QUrl currentUrl);
    QUrl currentUrl();
    void setCurrentSecurity(SecurityType securityType);
    SecurityType CurrentSecurity();
    void setHoverText(QString text);
    void clearHoverText();
signals:
    void GotFocus();

public slots:

private:
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void paintEvent(QPaintEvent* event);

    void updateText();

    QUrl url;
    QString hoverText;
    SecurityType sec = None;
};

#endif // SPACESEARCHBOX_H
