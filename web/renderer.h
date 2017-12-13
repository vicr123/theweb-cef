#ifndef RENDERER_H
#define RENDERER_H

#include <QWidget>
#include <include/cef_browser.h>
#include <include/cef_render_handler.h>
#include <QPainter>
#include <QResizeEvent>

#ifdef Q_OS_LINUX
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/XKBlib.h>
#include <QX11Info>

#undef None
#undef Status
#undef Unsorted
#undef Bool
#endif

class Renderer : public QWidget
{
        Q_OBJECT
    public:
        explicit Renderer(CefRefPtr<CefBrowser> browser, QWidget *parent = nullptr);

        QRect RootScreenRect();
        void paint(CefRenderHandler::PaintElementType type, QList<QRect> dirty, const void *buffer, int width, int height);
        void setCursor(CefCursorHandle cursor, CefRenderHandler::CursorType type, const CefCursorInfo& custom_cursor_info);
    signals:

    public slots:
        void pause();
        void resume();

    private:
        CefRefPtr<CefBrowser> browser;
        CefRefPtr<CefBrowserHost> host;
        QImage image;
        QPainter* mainPainter;

        bool paused = false;

        void paintEvent(QPaintEvent* event);
        void resizeEvent(QResizeEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        void mouseDoubleClickEvent(QMouseEvent* event);
        void leaveEvent(QEvent* event);
        void keyPressEvent(QKeyEvent* event);
        void keyReleaseEvent(QKeyEvent* event);
        void focusInEvent(QFocusEvent* event);
        void focusOutEvent(QFocusEvent* event);
        void wheelEvent(QWheelEvent* event);
};

#endif // RENDERER_H
