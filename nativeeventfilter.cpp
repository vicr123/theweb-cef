#include "nativeeventfilter.h"

NativeEventFilter::NativeEventFilter(QObject *parent) : QObject(parent)
{

}

bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        if (event->response_type == XCB_KEY_RELEASE) {
            //if (lastPress.restart() > 100) {
                xcb_key_release_event_t* button = static_cast<xcb_key_release_event_t*>(message);

                if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPlay) || button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioStop)) {
                    emit PlayPause();
                }
            //}
        }
    }
    return false;
}
