#ifndef BROWSERTAB_H
#define BROWSERTAB_H

#include <QPushButton>
#include <QLabel>
#include <include/cef_browser.h>
#include "cef/client.h"
#include <QWindow>
#include <QStackedLayout>
#include <QGraphicsOpacityEffect>
#include "browserdialog.h"
#include "renderer.h"
#include "mainwindow.h"
#include "tabbutton.h"

class MainWindow;
class BrowserDialog;

class BrowserTab : public QWidget
{
    Q_OBJECT

    public:
        explicit BrowserTab(MainWindow* parent, CefRefPtr<CefBrowser> browser = nullptr);
        ~BrowserTab();

        TabButton* getTabButton();
        Renderer* getRenderer();
        CefRefPtr<CefBrowser> getBrowser();
        BrowserDialog* getDialog();
        MainWindow* getMainWindow();

        void setTitle(QString title);
        void loadStateChange(bool isLoading, bool canGoBack, bool canGoForward);
        void setFullScreen(bool isFullScreen);
        void newAddress(QString address);

    signals:
        void activate();
        void addressChange(QString address);

    public slots:
        void requestClose();
        void reload();
        void contextMenu(CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback);
        void statusMessage(QString message);

    private:
        QStackedLayout* layout;
        TabButton* tabButton;
        CefRefPtr<CefBrowser> browser;
        Renderer* renderer = nullptr;
        MainWindow* mainWindow;
        BrowserDialog* dialog;
        QFrame* statusPanel = nullptr;
        QLabel* status = nullptr;
        QGraphicsOpacityEffect* statusEffect;

        void resizeEvent(QResizeEvent* event);
};

#endif // BROWSERTAB_H
