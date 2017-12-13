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
    windowInfo.SetAsWindowless(this->window()->winId());
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

    tToast* toast = new tToast();
    toast->setText(tr("Full Screen"));
    toast->setText(tr("This website is now full screen."));
    toast->show(mainWindow);
    connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
}

void BrowserTab::resizeEvent(QResizeEvent* event) {
    dialog->resize(this->size());
}
