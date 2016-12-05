#include "hovertabbar.h"

extern CefHandler* handler;
extern CefBrowserSettings defaultBrowserSettings;

HoverTabBar::HoverTabBar(MainWindow *parent) : QTabBar(parent)
{
    this->setMouseTracking(true);
    this->setStyleSheet("");
    this->setAcceptDrops(true);
    this->mainWindow = parent;
}

void HoverTabBar::mouseMoveEvent(QMouseEvent *event) {
    if (settings.value("behaviour/tabPreview", false).toBool()) {
        int tabAtIndex = tabAt(event->pos());
        if (currentHoverTab != tabAtIndex) {
            if (tabAtIndex != this->currentIndex() && tabAtIndex != -1) {
                emit previewTab(tabAtIndex);
            } else if (tabAtIndex == this->currentIndex()) {
                emit this->cancelPreview();
            }
            currentHoverTab = tabAtIndex;
        }
    }

    event->accept();

}

void HoverTabBar::leaveEvent(QEvent *event) {
    //Don't check the setting here (just in case)
    emit this->cancelPreview();
    currentHoverTab = -1;
}

void HoverTabBar::setStyleSheet(QString styleSheet) {
    //Check if this is a refresh or not
    if (styleSheet != "") {
        //We need to set the new stylesheet
        currentStyleSheet = styleSheet;
    }

    QString newStyleSheet = "QTabBar::scroller {width: 0px;}";
    QTabBar::setStyleSheet(newStyleSheet + currentStyleSheet);
}

void HoverTabBar::dragEnterEvent(QDragEnterEvent* event) {
    const QMimeData* data = event->mimeData();
    if (data->hasUrls()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void HoverTabBar::dragMoveEvent(QDragMoveEvent* event) {
    int tabAtIndex = tabAt(event->pos());

}

void HoverTabBar::dragLeaveEvent(QDragLeaveEvent* event) {

}

void HoverTabBar::dropEvent(QDropEvent* event) {
    const QMimeData* data = event->mimeData();
    int tabAtIndex = tabAt(event->pos());

    if (data->hasUrls() && data->urls().count() > 0) {
        //Direction:
        //0: None
        //1: Left
        //2: Right

        int direction = 0;
        //Go 5 pixels in both directions so we can see where to open this new tab
        int x = event->pos().x();
        while (x - 5 != event->pos().x()) {
            x++;
            if (tabAt(QPoint(x, event->pos().y())) != tabAtIndex) {
                direction = 2;
                break;
            }
        }

        if (direction == 0) {
            x = event->pos().x();
            while (x + 5 != event->pos().x()) {
                x--;
                if (tabAt(QPoint(x, event->pos().y())) != tabAtIndex) {
                    direction = 1;
                    break;
                }
            }
        }

        QString url = data->urls().first().toString();

        if (direction == 0) {
            //Spawn tab and replace
            //mainWindow->navigate(tabAtIndex, url);
            //mainWindow->browserList.at(index).get()->GetMainFrame().get()->LoadURL(url.toStdString());

            qDebug() << "Spawn Browser and replace tab";
        } else {
            Browser newBrowser;
            if (mainWindow->oblivionWindow()) {
                CefBrowserSettings settings = defaultBrowserSettings;
                settings.application_cache = STATE_DISABLED;

                CefRequestContextSettings contextSettings;
                CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(contextSettings, new OblivionRequestContextHandler);
                context.get()->RegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
                newBrowser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, url.toStdString(), settings, context);
            } else {
                newBrowser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, url.toStdString(), defaultBrowserSettings, CefRefPtr<CefRequestContext>());
            }

            if (direction == 1) {
                //Spawn tab on left
                mainWindow->createNewTab(newBrowser);
                qDebug() << "Load on left";
            } else {
                //Spawn tab on right
                mainWindow->createNewTab(newBrowser);
                qDebug() << "Load on right";
            }
        }
    }
}
