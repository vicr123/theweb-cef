#include "browsertab.h"

extern Client* cefClient;

BrowserTab::BrowserTab(MainWindow* parent, CefRefPtr<CefBrowser> browser) : QWidget(parent)
{
    tabButton = new TabButton;
    tabButton->setText("New Tab");
    connect(tabButton, SIGNAL(clicked(bool)), this, SIGNAL(activate()));

    cefClient->registerTab(this);

    CefWindowInfo windowInfo;
    CefBrowserSettings browserSettings;
    windowInfo.SetAsWindowless(NULL /*this->window()->winId()*/);
    browserSettings.background_color = 0xFFFFFFFF;

    if (browser == nullptr) {
        //this->browser = CefBrowserHost::CreateBrowserSync(windowInfo, cefClient, "http://vicr123.com/", browserSettings, CefRefPtr<CefRequestContext>());
    } else {
        this->browser = browser;
    }

    renderer = new Renderer(this->browser);
    mainWindow = parent;

    //QWindow* window = QWindow::fromWinId(browser.get()->GetHost().get()->GetWindowHandle());
    //QWidget* embeddedWidget = QWidget::createWindowContainer(window);

    layout = new QStackedLayout();
    //layout->addWidget(embeddedWidget);
    layout->addWidget(renderer);
    //layout->addWidget(dialog);
    layout->setMargin(0);
    this->setLayout(layout);

    dialog = new BrowserDialog();
    dialog->setParent(this);
    dialog->setVisible(false);
    dialog->move(0, 0);
    dialog->resize(this->size());
    dialog->raise();

    connect(dialog, &BrowserDialog::dismiss, [=] {
        //layout->setCurrentIndex(0);
        dialog->setVisible(false);
        renderer->resume();
    });
    connect(dialog, &BrowserDialog::activate, [=] {
        //layout->setCurrentIndex(1);
        dialog->setVisible(true);
        renderer->pause();
    });
}

BrowserTab::~BrowserTab() {
    tabButton->setVisible(false);
    tabButton->deleteLater();
    renderer->deleteLater();
}

void BrowserTab::reload() {
    dialog->dismiss();
    browser.get()->Reload();
}

void BrowserTab::newAddress(QString address) {
    dialog->dismiss();
    addressChange(address);
}

TabButton* BrowserTab::getTabButton() {
    return tabButton;
}

CefRefPtr<CefBrowser> BrowserTab::getBrowser() {
    return browser;
}

Renderer* BrowserTab::getRenderer() {
    return renderer;
}

BrowserDialog* BrowserTab::getDialog() {
    return dialog;
}

MainWindow* BrowserTab::getMainWindow() {
    return mainWindow;
}

void BrowserTab::requestClose() {
    browser.get()->GetHost().get()->TryCloseBrowser();
}

void BrowserTab::setTitle(QString title) {
    tabButton->setText(title);
}

void BrowserTab::loadStateChange(bool isLoading, bool canGoBack, bool canGoForward) {
    tabButton->setLoading(isLoading);
}

void BrowserTab::setFullScreen(bool isFullScreen) {
    mainWindow->setFullScreen(this, isFullScreen);

#ifdef Q_OS_LINUX
    tToast* toast = new tToast();
    toast->setText(tr("Full Screen"));
    toast->setText(tr("This website is now full screen."));
    toast->show(mainWindow);
    connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
#endif
}

void BrowserTab::resizeEvent(QResizeEvent* event) {
    dialog->resize(this->size());
}

void BrowserTab::contextMenu(CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback) {
    QMenu* menu = new QMenu();
    bool selectedItem = false;
    for (int i = 0; i < model.get()->GetCount(); i++) {
        int commandId = model.get()->GetCommandIdAt(i);
        switch (model.get()->GetType(commandId)) {
            case MENUITEMTYPE_COMMAND: {
                QString icon;
                switch (commandId) {
                    case MENU_ID_BACK:
                        icon = "go-previous";
                        break;
                    case MENU_ID_FORWARD:
                        icon = "go-next";
                        break;
                    case MENU_ID_RELOAD_NOCACHE:
                        icon = "view-refresh";
                        break;
                    case MENU_ID_SPELLCHECK_SUGGESTION_0:
                    case MENU_ID_SPELLCHECK_SUGGESTION_1:
                    case MENU_ID_SPELLCHECK_SUGGESTION_2:
                    case MENU_ID_SPELLCHECK_SUGGESTION_3:
                    case MENU_ID_SPELLCHECK_SUGGESTION_4:
                    case MENU_ID_NO_SPELLING_SUGGESTIONS:
                        icon = "tools-check-spelling";
                        break;
                    case MENU_ID_ADD_TO_DICTIONARY:
                        icon = "list-add";
                        break;
                    case MENU_ID_CUT:
                        icon = "edit-cut";
                        break;
                    case MENU_ID_PASTE:
                        icon = "edit-paste";
                        break;
                    case MENU_ID_UNDO:
                        icon = "edit-undo";
                        break;
                    case MENU_ID_REDO:
                        icon = "edit-redo";
                        break;
                    case MENU_ID_SELECT_ALL:
                        icon = "edit-select-all";
                        break;
                    case MENU_ID_DELETE:
                        icon = "edit-delete";
                        break;
                    case MENU_ID_COPY:
                    case Client::CopyLink:
                        icon = "edit-copy";
                        break;
                    case Client::OpenLinkInNewTab:
                        icon = "tab-new";
                        break;
                    case Client::OpenLinkInNewWindow:
                    case Client::OpenLinkInNewOblivion:
                        icon = "window-new";
                        break;
                }

                QAction* action = new QAction();
                action->setText(QString::fromStdString(model.get()->GetLabel(commandId).ToString()));
                action->setIcon(QIcon::fromTheme(icon));
                action->setEnabled(model.get()->IsEnabled(commandId));
                connect(action, &QAction::triggered, [callback, commandId, &selectedItem] {
                    selectedItem = true;
                    callback.get()->Continue(commandId, EVENTFLAG_NONE);
                });
                menu->addAction(action);
                break;
            }
            case MENUITEMTYPE_SEPARATOR:
                menu->addSeparator();
                break;
            case MENUITEMTYPE_SUBMENU:
                menu->addSection(QString::fromStdString(model.get()->GetLabel(commandId).ToString()));
                break;
        }
    }
    connect(menu, &QMenu::destroyed, [callback, &selectedItem] {
        if (selectedItem == false) {
            callback.get()->Cancel();
        }
    });
    connect(menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));
    menu->exec(mapToGlobal(QPoint(params.get()->GetXCoord(), params.get()->GetYCoord())));
}

void BrowserTab::statusMessage(QString message) {
    if (status != nullptr) {
        //Animate old label out
        tPropertyAnimation* anim1 = new tPropertyAnimation(statusEffect, "opacity");
        anim1->setStartValue((float) 1);
        anim1->setEndValue((float) 0);
        anim1->setEasingCurve(QEasingCurve::OutCubic);
        anim1->setDuration(500);
        connect(anim1, SIGNAL(finished()), anim1, SLOT(deleteLater()));
        connect(anim1, SIGNAL(finished()), statusEffect, SLOT(deleteLater()));
        anim1->start();

        tPropertyAnimation* anim2 = new tPropertyAnimation(status, "geometry");
        anim2->setStartValue(status->geometry());
        anim2->setEndValue(status->geometry().adjusted(0, -status->fontMetrics().height(), 0, -status->fontMetrics().height()));
        anim2->setEasingCurve(QEasingCurve::OutCubic);
        anim2->setDuration(500);
        connect(anim2, SIGNAL(finished()), anim2, SLOT(deleteLater()));
        connect(anim2, SIGNAL(finished()), status, SLOT(deleteLater()));
        anim2->start();

        status = nullptr;
    }

    if (message != "") {
        //Animate new label in
        status = new QLabel();
        status->setParent(this);
        status->setVisible(true);
        status->setText(message);
        status->raise();
        status->setAutoFillBackground(true);
        status->setAttribute(Qt::WA_TransparentForMouseEvents);

        QRect geom;
        geom.setLeft(9);
        geom.setWidth(status->fontMetrics().width(message));
        geom.setHeight(status->fontMetrics().height());
        geom.moveTop(this->height() - 9);
        geom.adjust(-9, -9, 9, 9);
        status->setGeometry(geom);

        statusEffect = new QGraphicsOpacityEffect();
        statusEffect->setOpacity(0);
        status->setGraphicsEffect(statusEffect);

        tPropertyAnimation* anim1 = new tPropertyAnimation(statusEffect, "opacity");
        anim1->setStartValue((float) 0);
        anim1->setEndValue((float) 1);
        anim1->setEasingCurve(QEasingCurve::OutCubic);
        anim1->setDuration(500);
        connect(anim1, SIGNAL(finished()), anim1, SLOT(deleteLater()));
        anim1->start();

        geom.moveTop(this->height() - 9 - status->fontMetrics().height());
        tPropertyAnimation* anim2 = new tPropertyAnimation(status, "geometry");
        anim2->setStartValue(status->geometry());
        anim2->setEndValue(geom);
        anim2->setEasingCurve(QEasingCurve::OutCubic);
        anim2->setDuration(500);
        connect(anim2, SIGNAL(finished()), anim2, SLOT(deleteLater()));
        anim2->start();
    }
}
