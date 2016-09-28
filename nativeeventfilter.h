#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <QX11Info>

class NativeEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit NativeEventFilter(QObject *parent = 0);

signals:
    void PlayPause();

public slots:

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
};

#endif // NATIVEEVENTFILTER_H
