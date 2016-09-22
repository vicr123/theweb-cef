#ifndef SPACESEARCHBOX_H
#define SPACESEARCHBOX_H

#include <QLineEdit>
#include <QFocusEvent>
#include <QUrl>
#include <QPaintEvent>
#include <QDebug>

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
signals:
    void GotFocus();

public slots:

private:
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);

    void updateText();

    QUrl url;
    SecurityType sec = None;
};

#endif // SPACESEARCHBOX_H
