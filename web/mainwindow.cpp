#include "mainwindow.h"
#include "ui_mainwindow.h"

#define TOQSTR(cefstr) QString::fromStdString(cefstr.ToString())

extern Client* cefClient;
extern CefBrowserSettings::struct_type browserSettings;

MainWindow::MainWindow(CefRefPtr<CefBrowser> browser, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->menuBar->setVisible(false);

    addressBar = new Bar;
    ui->mainToolBar->addWidget(addressBar);
    connect(addressBar, &QLineEdit::returnPressed, [=] {
        CURRENT_TAB->getBrowser().get()->GetMainFrame().get()->LoadURL(addressBar->text().toStdString());
    });

    createNewTab(browser);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createNewTab(CefRefPtr<CefBrowser> browser) {
    BrowserTab* tab = new BrowserTab(this, browser);
    ((QBoxLayout*) ui->tabScrollContents->layout())->addWidget(tab->getTabButton());
    ui->tabs->addWidget(tab);

    connect(tab, &BrowserTab::destroyed, [=] {
        ui->tabs->removeWidget(tab);
        if (ui->tabs->count() == 0) {
            this->close();
        }
    });
    connect(tab, &BrowserTab::addressChange, [=](QString newAddress) {
        if (CURRENT_TAB == tab) {
            addressBar->setUrl(QUrl(newAddress));
        }
    });
    connect(tab, &BrowserTab::activate, [=] {
        ui->tabs->setCurrentWidget(tab);
    });
}

void MainWindow::on_tabs_currentChanged(int arg1)
{
    if (arg1 == -1) return;
    BrowserTab* tab = (BrowserTab*) ui->tabs->widget(arg1);
    addressBar->setUrl(QUrl(TOQSTR(tab->getBrowser().get()->GetMainFrame().get()->GetURL())));

    for (int i = 0; i < ui->tabs->count(); i++) {
        QPushButton* tabButton = ((BrowserTab*) ui->tabs->widget(i))->getTabButton();
        if (i == arg1) {
            tabButton->setCheckable(true);
            tabButton->setChecked(true);
        } else {
            tabButton->setCheckable(false);
            tabButton->setChecked(false);
        }
        tabButton->repaint();
    }
}

void MainWindow::on_newTabButton_clicked()
{
    cefClient->setNewTabWindow(this);

    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(NULL);
    CefBrowserHost::CreateBrowser(windowInfo, cefClient, "http://www.google.com/", browserSettings, CefRefPtr<CefRequestContext>());
}

void MainWindow::on_closeTabButton_clicked()
{
    BrowserTab* tab = CURRENT_TAB;
    tab->requestClose();
}

void MainWindow::on_actionBack_triggered()
{
    CURRENT_TAB->getBrowser().get()->GoBack();
}

void MainWindow::on_actionForward_triggered()
{
    CURRENT_TAB->getBrowser().get()->GoForward();
}

void MainWindow::setFullScreen(BrowserTab* tab, bool isFullScreen) {
    if (isFullScreen) {
        this->showFullScreen();
        ui->mainToolBar->setVisible(false);
        ui->tabBar->setVisible(false);
        ui->tabs->setCurrentWidget(tab);
    } else {
        this->showNormal();
        ui->mainToolBar->setVisible(true);
        ui->tabBar->setVisible(true);
    }
}

void MainWindow::on_actionReload_triggered()
{
    CURRENT_TAB->reload();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (ui->tabs->count() == 0) {
        event->accept();
        return;
    }

    for (int i = 0; i < ui->tabs->count(); i++) {
        ((BrowserTab*) ui->tabs->widget(i))->requestClose();
    }
    event->ignore();
}
