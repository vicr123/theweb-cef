#include "mainwindow.h"
#include "ui_mainwindow.h"

extern CefHandler* handler;
extern SignalBroker* signalBroker;
extern void QuitApp(int exitCode = 0);
extern QTimer cefEventLoopTimer;
extern QStringList certErrorUrls;
extern QTimer batteryCheckTimer;
extern QFile historyFile;
extern CefBrowserSettings defaultBrowserSettings;

MainWindow::MainWindow(Browser newBrowser, bool isOblivion, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->isOblivion = isOblivion;

    //Set up tab bar
    tabBar = new HoverTabBar();
    tabBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    tabBar->setTabsClosable(true);
    tabBar->setShape(QTabBar::RoundedNorth);
    tabBar->setExpanding(false);
    tabBar->setMovable(true);
    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    ((QBoxLayout*) ui->centralwidget->layout())->insertWidget(ui->centralwidget->layout()->indexOf(ui->errorFrame), tabBar);
    connect(tabBar, SIGNAL(previewTab(int)), ui->browserStack, SLOT(previewTab(int)));
    connect(tabBar, SIGNAL(cancelPreview()), ui->browserStack, SLOT(cancelPreview()));
    connect(tabBar, &QTabBar::customContextMenuRequested, [=](QPoint pos) {
        int tab = tabBar->tabAt(pos);
        QMenu* menu = new QMenu;
        menu->addSection("For tab \"" + tabBar->tabData(tab).toString() + "\"");
        menu->addAction(QIcon::fromTheme("tab-close"), "Close Tab", [=]() {
            browserList.at(tab).get()->GetHost().get()->CloseBrowser(false);
        });
        menu->addSection("General");
        menu->addAction(QIcon::fromTheme("tab-new"), "New Tab", [=]() {
            createNewTab();
        });
        menu->exec(tabBar->mapToGlobal(pos));
        menu->deleteLater();
    });

    //Set up loading animation
    QString resourceName;
    QColor panelColor = ui->securityFrame->palette().color(QPalette::Window);
    if (((qreal) panelColor.red() + (qreal) panelColor.green() + (qreal) panelColor.red()) / (qreal) 3 < 127) {
        resourceName = ":/icons/load-d";
    } else {
        resourceName = ":/icons/load-l";
    }

    tabLoading = new QMovie(resourceName);
    connect(tabLoading, &QMovie::frameChanged, [=]() {
        QIcon icon(tabLoading->currentPixmap());
        int i = 0;
        for (Browser browser : browserList) {
            if (browser.get()->IsLoading()) {
                tabBar->setTabIcon(i, icon);
            }
            i++;
        }
    });
    tabLoading->start();

    //Setup UI
    this->resize(700, 700);
    ui->errorFrame->setVisible(false);
    ui->warningFrame->setVisible(false);
    ui->menubar->setVisible(false);
    ui->JsDialogFrame->setVisible(false);
    ui->AuthFrame->setVisible(false);
    ui->fraudFrame->setVisible(false);
    ui->fraudExtraFrame->setVisible(false);
    ui->securityEVName->setVisible(false);
    ui->securityFrame->setFixedHeight(ui->securityEVName->height());
    ui->badCertificateFrame->setVisible(false);
    ui->certMoreInfo->setVisible(false);
    ui->hoverUrlLabel->setVisible(false);
    ui->downloadItemAreaWidget->setVisible(false);
    ui->findPanel->setVisible(false);
    ui->SelectFileFrame->setVisible(false);
    ui->printFrame->setVisible(false);

    //Set flat theme when Contemporary theme is used
    ui->JsDialogOk->setProperty("flatContemporary", true);
    ui->JsDialogCancel->setProperty("flatContemporary", true);
    ui->AuthOk->setProperty("flatContemporary", true);
    ui->AuthCancel->setProperty("flatContemporary", true);
    ui->reloadErrorButton->setProperty("type", "positive");
    ui->certificateBack->setProperty("type", "positive");
    ui->fraudBack->setProperty("type", "positive");

    ui->hoverUrlLabel->setParent(this);
    ui->hoverUrlLabel->move(10, this->height() - ui->hoverUrlLabel->height() - 10);

    //Is this user root?
    if (geteuid() == 0) {
        //Warn user that they're running as root
        currentWarning = MainWindow::warning;
        ui->warningLabel->setText("theWeb is being run as root. As a result, the sandbox has been disabled. You should not run theWeb as the root user.");

        ui->warningOk->setVisible(true);
        ui->warningCancel->setVisible(false);
        ui->warningOk->setText("OK");
        ui->warningFrame->setVisible(true);
    }

    QPalette oldFraudContentPalette = ui->fraudContent->palette();
    oldFraudContentPalette.setColor(QPalette::Window, oldFraudContentPalette.color(QPalette::Window));
    QPalette oldFraudPalette = ui->fraudFrame->palette();
    oldFraudPalette.setColor(QPalette::Window, QColor::fromRgb(100, 0, 0));
    ui->fraudFrame->setPalette(oldFraudPalette);
    ui->badCertificateFrame->setPalette(oldFraudPalette);
    ui->fraudContent->setPalette(oldFraudContentPalette);
    ui->badCertificateFrameContent->setPalette(oldFraudContentPalette);

    ui->securityPadlock->setPixmap(QIcon::fromTheme("text-html").pixmap(16, 16));

    ui->toolBar->addWidget(ui->securityFrame);
    ui->toolBar->addWidget(ui->spaceSearch);

    ui->securityFrame->setAutoFillBackground(true);

    QMenu* menu = new QMenu();
    menu->addAction(ui->actionNew_Tab);
    menu->addAction(ui->actionNew_Window);
    menu->addAction(ui->actionNew_Oblivion_Window);
    menu->addSeparator();
    menu->addAction(ui->actionFind);
    menu->addSeparator();
    menu->addAction(ui->actionPrint);
    menu->addSeparator();
    menu->addAction(ui->actionHistory);
    menu->addAction(ui->actionSettings);
    menu->addAction(ui->actionAbout_theWeb);
    menu->addSeparator();
    menu->addAction(ui->actionClose_Tab);
    menu->addAction(ui->actionExit);

    menuButton = new QToolButton();
    menuButton->setPopupMode(QToolButton::InstantPopup);
    if (isOblivion) {
        menuButton->setIcon(QIcon(":/icons/oblivionIcon"));
        QPalette searchPalette = ui->spaceSearch->palette();
        QColor temp;
        temp = searchPalette.color(QPalette::Base);
        searchPalette.setColor(QPalette::Base, searchPalette.color(QPalette::Text));
        searchPalette.setColor(QPalette::Text, temp);

        temp = searchPalette.color(QPalette::Window);
        searchPalette.setColor(QPalette::Window, searchPalette.color(QPalette::WindowText));
        searchPalette.setColor(QPalette::WindowText, temp);

        ui->spaceSearch->setPalette(searchPalette);
    } else {
        menuButton->setIcon(QIcon(":/icons/icon"));
    }
    menuButton->setMenu(menu);
    menuButtonAction = ui->toolBar->insertWidget(ui->actionGo_Back, menuButton);

    QMenu* powerMenu = new QMenu();
    powerMenu->addSection("Framerate limiting");
    powerMenu->addAction(ui->actionDon_t_Limit);
    powerMenu->addAction(ui->actionLimit_to_60_fps);
    powerMenu->addAction(ui->actionLimit_to_30_fps);
    powerMenu->addAction(ui->actionLimit_to_15_fps);
    powerMenu->addAction(ui->actionLimit_to_1_fps);

    QToolButton* powerButton = new QToolButton();
    powerButton->setPopupMode(QToolButton::InstantPopup);
    powerButton->setIcon(QIcon::fromTheme("battery"));
    powerButton->setMenu(powerMenu);
    ui->toolBar->addWidget(powerButton);

    //Connect signals and slots from the signal broker. Usually these are sent from the CEF Handler.
    connect(signalBroker, SIGNAL(RenderProcessTerminated(Browser,CefRequestHandler::TerminationStatus)), this, SLOT(RenderProcessTerminated(Browser,CefRequestHandler::TerminationStatus)));
    connect(signalBroker, SIGNAL(TitleChanged(Browser,CefString)), this, SLOT(TitleChanged(Browser,CefString)));
    connect(signalBroker, SIGNAL(AddressChange(Browser,CefRefPtr<CefFrame>,CefString)), this, SLOT(AddressChange(Browser,CefRefPtr<CefFrame>,CefString)));
    connect(signalBroker, SIGNAL(FullscreenModeChange(Browser,bool)), this, SLOT(FullscreenModeChange(Browser,bool)));
    connect(signalBroker, SIGNAL(JSDialog(CefRefPtr<CefBrowser>,CefString,CefHandler::JSDialogType,CefString,CefString,CefRefPtr<CefJSDialogCallback>,bool&)), this, SLOT(JSDialog(CefRefPtr<CefBrowser>,CefString,CefHandler::JSDialogType,CefString,CefString,CefRefPtr<CefJSDialogCallback>,bool&)));
    connect(signalBroker, SIGNAL(LoadingStateChange(Browser,bool,bool,bool)), this, SLOT(LoadingStateChange(Browser,bool,bool,bool)));
    connect(signalBroker, SIGNAL(LoadError(Browser,CefRefPtr<CefFrame>,CefHandler::ErrorCode,CefString,CefString)), this, SLOT(LoadError(Browser,CefRefPtr<CefFrame>,CefHandler::ErrorCode,CefString,CefString)));
    connect(signalBroker, SIGNAL(BeforeClose(Browser)), this, SLOT(BeforeClose(Browser)));
    connect(signalBroker, SIGNAL(AuthCredentials(Browser,CefRefPtr<CefFrame>,bool,CefString,int,CefString,CefString,CefRefPtr<CefAuthCallback>)), this, SLOT(AuthCredentials(Browser,CefRefPtr<CefFrame>,bool,CefString,int,CefString,CefString,CefRefPtr<CefAuthCallback>)));
    connect(signalBroker, SIGNAL(BeforeUnloadDialog(Browser,CefString,bool,CefRefPtr<CefJSDialogCallback>)), this, SLOT(BeforeUnloadDialog(Browser,CefString,bool,CefRefPtr<CefJSDialogCallback>)));
    connect(signalBroker, SIGNAL(BeforePopup(Browser,CefRefPtr<CefFrame>,CefString,CefString,CefLifeSpanHandler::WindowOpenDisposition,bool,CefPopupFeatures,CefWindowInfo*,CefBrowserSettings,bool*)), this, SLOT(BeforePopup(Browser,CefRefPtr<CefFrame>,CefString,CefString,CefLifeSpanHandler::WindowOpenDisposition,bool,CefPopupFeatures,CefWindowInfo*,CefBrowserSettings,bool*)));
    connect(signalBroker, SIGNAL(CertificateError(Browser,cef_errorcode_t,CefString,CefRefPtr<CefSSLInfo>,CefRefPtr<CefRequestCallback>)), this, SLOT(CertificateError(Browser,cef_errorcode_t,CefString,CefRefPtr<CefSSLInfo>,CefRefPtr<CefRequestCallback>)));
    connect(signalBroker, SIGNAL(FaviconURLChange(Browser,std::vector<CefString>)), this, SLOT(FaviconURLChange(Browser,std::vector<CefString>)));
    connect(signalBroker, SIGNAL(KeyEvent(CefRefPtr<CefBrowser>,CefKeyEvent,XEvent*)), this, SLOT(KeyEvent(CefRefPtr<CefBrowser>,CefKeyEvent,XEvent*)));
    connect(signalBroker, SIGNAL(ContextMenu(Browser,CefRefPtr<CefFrame>,CefRefPtr<CefContextMenuParams>,CefRefPtr<CefMenuModel>,CefRefPtr<CefRunContextMenuCallback>)), this, SLOT(ContextMenu(Browser,CefRefPtr<CefFrame>,CefRefPtr<CefContextMenuParams>,CefRefPtr<CefMenuModel>,CefRefPtr<CefRunContextMenuCallback>)));
    connect(signalBroker, SIGNAL(ContextMenuCommand(Browser,int,CefRefPtr<CefContextMenuParams>)), this, SLOT(ContextMenuCommand(Browser,int,CefRefPtr<CefContextMenuParams>)));
    connect(signalBroker, SIGNAL(ProtocolExecution(Browser,CefString,bool&)), this, SLOT(ProtocolExecution(Browser,CefString,bool&)));
    connect(signalBroker, SIGNAL(Tooltip(Browser,CefString&)), this, SLOT(Tooltip(Browser,CefString&)));
    connect(signalBroker, SIGNAL(ShowBrowser(Browser)), this, SLOT(ShowBrowser(Browser)));
    connect(signalBroker, SIGNAL(AskForNotification(Browser,CefString)), this, SLOT(AskForNotification(Browser,CefString)));
    connect(signalBroker, SIGNAL(MprisStateChanged(Browser,bool)), this, SLOT(MprisStateChanged(Browser,bool)));
    connect(signalBroker, SIGNAL(MprisPlayingStateChanged(Browser,bool)), this, SLOT(MprisPlayingStateChanged(Browser,bool)));
    connect(signalBroker, SIGNAL(BeforeDownload(Browser,CefRefPtr<CefDownloadItem>,CefString,CefRefPtr<CefBeforeDownloadCallback>)), this, SLOT(BeforeDownload(Browser,CefRefPtr<CefDownloadItem>,CefString,CefRefPtr<CefBeforeDownloadCallback>)));
    connect(signalBroker, SIGNAL(NewDownload(Browser,CefRefPtr<CefDownloadItem>)), this, SLOT(NewDownload(Browser,CefRefPtr<CefDownloadItem>)));
    connect(signalBroker, SIGNAL(OpenURLFromTab(Browser,CefRefPtr<CefFrame>,CefString,CefLifeSpanHandler::WindowOpenDisposition,bool)), this, SLOT(OpenURLFromTab(Browser,CefRefPtr<CefFrame>,CefString,CefLifeSpanHandler::WindowOpenDisposition,bool)));
    connect(signalBroker, SIGNAL(FileDialog(Browser,CefDialogHandler::FileDialogMode,CefString,CefString,std::vector<CefString>,int,CefRefPtr<CefFileDialogCallback>)), this, SLOT(FileDialog(Browser,CefDialogHandler::FileDialogMode,CefString,CefString,std::vector<CefString>,int,CefRefPtr<CefFileDialogCallback>)));
    connect(signalBroker, SIGNAL(PrintDialog(Browser,QPrinter*,bool,CefRefPtr<CefPrintDialogCallback>)), this, SLOT(PrintDialog(Browser,QPrinter*,bool,CefRefPtr<CefPrintDialogCallback>)));
    connect(signalBroker, SIGNAL(StatusMessage(Browser,CefString)), this, SLOT(StatusMessage(Browser,CefString)));
    connect(signalBroker, SIGNAL(ReloadSettings()), this, SLOT(ReloadSettings()));

    createNewTab(newBrowser);

    //Connect tabBar signals down here because some things aren't initialized yet. :)
    connect(tabBar, &HoverTabBar::currentChanged, [=](int currentIndex) {
        if (currentIndex != -1) {
            Browser newBrowser = browserList.at(currentIndex);
            ui->browserStack->setCurrentIndex(currentIndex);
            ui->spaceSearch->setCurrentUrl(QUrl(QString::fromStdString(newBrowser.get()->GetMainFrame().get()->GetURL())));
        }

        updateCurrentBrowserDisplay();
    });
    connect(tabBar, &HoverTabBar::tabCloseRequested, [=](int closeIndex) {
        browserList.at(closeIndex).get()->GetHost().get()->CloseBrowser(false);
    });
    connect(tabBar, &HoverTabBar::tabMoved, [=](int from, int to) {
        //Update all required lists
        browserList.move(from, to);
        browserMetadata.move(from, to);
        updateCurrentBrowserDisplay();
    });
    ui->browserStack->lower();
    ReloadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_Tab_triggered()
{
    createNewTab();
}

void MainWindow::createNewTab(Browser newBrowser, bool openInBackground) {
    browserMetadata.append(QVariantMap());
    browserIcons.append(QIcon::fromTheme("text-html"));
    tabBar->addTab("New Tab");

    CefWindowInfo windowInfo;
    CefBrowserSettings settings = defaultBrowserSettings;

    if (isOblivion) {
        settings.application_cache = STATE_DISABLED;
    }

    Browser browser;
    if (newBrowser.get() == 0) {
        if (isOblivion) {
            CefRequestContextSettings contextSettings;
            CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(contextSettings, new OblivionRequestContextHandler);
            context.get()->RegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
            browser = CefBrowserHost::CreateBrowserSync(windowInfo, CefRefPtr<CefHandler>(handler), "theweb://oblivion", settings, context);
        } else {
            browser = CefBrowserHost::CreateBrowserSync(windowInfo, CefRefPtr<CefHandler>(handler), this->settings.value("browser/home", "theweb://newtab").toString().toStdString(), settings, CefRefPtr<CefRequestContext>());
        }
    } else {
        browser = newBrowser;
    }

    QWindow* window = QWindow::fromWinId(browser.get()->GetHost()->GetWindowHandle());
    QWidget* browserWidget = QWidget::createWindowContainer(window);
    browserWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    ui->browserStack->addWidget(browserWidget);
    browserList.append(browser);

    if (!openInBackground) {
        ui->browserStack->setCurrentIndex(tabBar->count() - 1, true);
        tabBar->setCurrentIndex(tabBar->count() - 1);
        browserWidget->setFocus();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    for (Browser browser : browserList) {
        if (!handler->canClose(browser)) {
            browser.get()->GetHost().get()->CloseBrowser(false);
            event->ignore();
        }
    }
}

void MainWindow::on_actionExit_triggered()
{
    QuitApp();
}

void MainWindow::on_actionGo_Back_triggered()
{
    browser().get()->GoBack();
}

void MainWindow::on_actionGo_Forward_triggered()
{
    browser().get()->GoForward();
}

void MainWindow::on_reloadErrorButton_clicked()
{
    removeFromMetadata(browser(), "error");
    removeFromMetadata(browser(), "crash");
    ui->actionReload->trigger();
    ui->errorFrame->setVisible(false);
    ui->browserStack->setVisible(true);
}

void MainWindow::on_actionReload_triggered()
{
    browser().get()->ReloadIgnoreCache();
}

bool MainWindow::IsCorrectBrowser(Browser browser) {
    if (browser.get() != NULL && this->browser().get() != NULL) {
        return browser.get()->GetIdentifier() == this->browser().get()->GetIdentifier();
    } else {
        return false;
    }
}

void MainWindow::RenderProcessTerminated(Browser browser, CefRequestHandler::TerminationStatus status) {
    if (indexOfBrowser(browser) != -1) {
        QStringList crashMetadata;

        crashMetadata.append("Well, this is strange.");
        switch (status) {
        case TS_PROCESS_CRASHED:
            crashMetadata.append("The window crashed.");
            break;
        case TS_PROCESS_WAS_KILLED:
            crashMetadata.append("The window was closed by the system.");
            break;
        case TS_ABNORMAL_TERMINATION:
            crashMetadata.append("The window closed abnormally.");
        }

        insertIntoMetadata(browser, "crash", crashMetadata);

        if (settings.value("browser/tabText", false).toBool()) {
            tabBar->setTabText(indexOfBrowser(browser), tabBar->fontMetrics().elidedText("Well, this is strange...", Qt::ElideRight, 200));
        } else {
            tabBar->setTabText(indexOfBrowser(browser), "");
        }
        tabBar->setTabData(indexOfBrowser(browser), "Well, this is strange...");
        //tabBar->setTabToolTip(indexOfBrowser(browser), "Well, this is strange...");

        QIcon icon = QIcon::fromTheme("dialog-error");
        tabBar->setTabIcon(indexOfBrowser(browser), QIcon(icon));
        browserIcons.replace(indexOfBrowser(browser), icon);

        updateCurrentBrowserDisplay();
    }
}

void MainWindow::TitleChanged(Browser browser, const CefString& title) {
    if (indexOfBrowser(browser) != -1) {
        if (settings.value("browser/tabText", false).toBool()) {
            tabBar->setTabText(indexOfBrowser(browser), tabBar->fontMetrics().elidedText(QString::fromStdString(title.ToString()), Qt::ElideRight, 200));
        } else {
            tabBar->setTabText(indexOfBrowser(browser), "");
        }
        tabBar->setTabData(indexOfBrowser(browser), QString::fromStdString(title.ToString()));
        //tabBar->setTabToolTip(indexOfBrowser(browser), QString::fromStdString(title.ToString()));
        if (IsCorrectBrowser(browser)) {
            this->setWindowTitle(QString::fromStdString(title.ToString()).append(" - theWeb"));
        }

        if (browserMetadata.at(indexOfBrowser(browser)).value("addToHistory", false) == true) {
            //Reset flags
            insertIntoMetadata(browser, "addToHistory", false);

            //Append item to history
            //Space is used as the delimiter.
            QString historyLine = QString::fromStdString(browser.get()->GetMainFrame().get()->GetURL().ToString());
            historyLine.append(" " + QString::fromStdString(title).replace(" ", "%20"));
            historyLine.append(" " + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()));

            historyFile.write(historyLine.toUtf8() + "\n");
            historyFile.flush();
        }
    }
}

void MainWindow::on_spaceSearch_returnPressed()
{
    QString urlToLoad;
    if (ui->spaceSearch->text().contains(".") || ui->spaceSearch->text().contains("/") || ui->spaceSearch->text().contains("\\") || ui->spaceSearch->text().contains(":")) {
        QUrl urlParser = QUrl::fromUserInput(ui->spaceSearch->text());
        if (!urlParser.isEmpty() || urlParser.scheme() == "theweb" || urlParser.scheme() == "chrome") {
            urlToLoad = urlParser.toEncoded();
        } else {
            urlToLoad = "http://www.google.com/search#q=" + ui->spaceSearch->text().replace(" ", "+");
        }
    } else {
        urlToLoad = "http://www.google.com/search#q=" + ui->spaceSearch->text().replace(" ", "+");
    }
    browser().get()->GetMainFrame().get()->LoadURL(urlToLoad.toStdString());
}

void MainWindow::AddressChange(Browser browser, CefRefPtr<CefFrame> frame, const CefString &url) {
    if (indexOfBrowser(browser) != -1 && frame.get()->IsMain()) { //Make sure frame is in this window
        QVariantList securityMetadata;

        QUrl currentUrl(QString::fromStdString(url.ToString()));
        ui->fraudFrame->setVisible(false);
        ui->fraudExtraFrame->setVisible(false);
        ui->fraudIgnore->setText("More Info");
        ui->badCertificateFrame->setVisible(false);
        ui->certMoreInfo->setVisible(false);
        ui->certIgnore->setText("More Info");
        removeFromMetadata(browser, "certificate");
        removeFromMetadata(browser, "threat");

        ui->spaceSearch->setCurrentUrl(currentUrl);

        if (QUrl(QString::fromStdString(url.ToString())).scheme() == "theweb") {
            securityMetadata.append("theweb");
            securityMetadata.append("theWeb Generated Content");
            /*securityMetadata.append("You are viewing content generated by theWeb.");*/
        } else {
            if (currentUrl.scheme() == "https") {
                if (certErrorUrls.contains(QString::fromStdString(url.ToString()))) {
                    securityMetadata.append("certerr");
                    securityMetadata.append("Insecure Connection");
                    /*securityMetadata.append("This connection may have been intercepted.");*/
                } else {
                    securityMetadata.append("secure");
                    securityMetadata.append("");
                    /*securityMetadata.append("Please wait...");*/

                    QSslSocket *sslSock = new QSslSocket();
                    QList<QNetworkProxy> proxyList = QNetworkProxyFactory::systemProxyForQuery();
                    if (proxyList.size() > 0) {
                        sslSock->setProxy(proxyList.first());
                    }
                    connect(sslSock, &QSslSocket::encrypted, [=]() {
                        QVariantList securityMetadata;

                        QSslCertificate certificate = sslSock->peerCertificate();
                        QList<QSslCertificate> certificateChain = sslSock->peerCertificateChain();
                        sslSock->close();
                        sslSock->deleteLater();

                        bool isEv = false;
                        //if (certificateChain.count() > 2) {

                            //List of EV Certificates that theWeb recognizes.
                            QMap<QString, QString> EVOids;
                            EVOids.insert("Actalis", "1.3.159.1.17.1");
                            EVOids.insert("AffirmTrust", "1.3.6.1.4.1.34697.2.1");
                            EVOids.insert("AffirmTrust", "1.3.6.1.4.1.34697.2.2");
                            EVOids.insert("AffirmTrust", "1.3.6.1.4.1.34697.2.3");
                            EVOids.insert("AffirmTrust", "1.3.6.1.4.1.34697.2.4");
                            EVOids.insert("A-Trust", "1.2.40.0.17.1.22");
                            EVOids.insert("Buypass Class 3 Root CA", "2.16.578.1.26.1.3.3");
                            EVOids.insert("AC Camerfirma S.A. Chambers of Commerce Root - 2008", "1.3.6.1.4.1.17326.10.14.2.1.2");
                            EVOids.insert("AC Camerfirma S.A. Global Chambersign Root - 2008", "1.3.6.1.4.1.17326.10.8.12.1.2");
                            EVOids.insert("COMODO SECURE™", "1.3.6.1.4.1.6449.1.2.1.5.1");
                            EVOids.insert("DigiCert", "2.16.840.1.114412.2.1");
                            EVOids.insert("Entrust.net", "2.16.840.1.114028.10.1.2");
                            EVOids.insert("GeoTrust Primary Certificate Authority - G3", "1.3.6.1.4.1.14370.1.6");
                            EVOids.insert("GlobalSign", "1.3.6.1.4.1.4146.1.1");
                            EVOids.insert("Go Daddy Root Certificate Authority – G2", "2.16.840.1.114413.1.7.23.3");
                            EVOids.insert("Izenpe", "1.3.6.1.4.1.14777.6.1.1");
                            EVOids.insert("Izenpe", "1.3.6.1.4.1.14777.6.1.2");
                            EVOids.insert("Keynectis", "1.3.6.1.4.1.22234.2.5.2.3.1");
                            EVOids.insert("Network Solutions", "1.3.6.1.4.1.782.1.2.1.8.1");
                            EVOids.insert("QuoVadis Root CA 2", "1.3.6.1.4.1.8024.0.2.100.1.2");
                            EVOids.insert("SECOM Trust Systems", "1.2.392.200091.100.721.1");
                            EVOids.insert("Starfield Root Certificate Authority – G2", "2.16.840.1.114414.1.7.23.3");
                            EVOids.insert("StartCom Certification Authority", "1.3.6.1.4.1.23223.2");
                            EVOids.insert("StartCom Certification Authority", "1.3.6.1.4.1.23223.1.1.1");
                            EVOids.insert("SwissSign", "2.16.756.1.89.1.2.1.1");
                            EVOids.insert("thawte Primary Root CA - G3", "2.16.840.1.113733.1.7.48.1");
                            EVOids.insert("Trustwave", "2.16.840.1.114404.1.1.2.4.1");
                            EVOids.insert("VeriSign Class 3 Public Primary Certification Authority - G5", "2.16.840.1.113733.1.7.23.6");
                            EVOids.insert("Verizon Business", "1.3.6.1.4.1.6334.1.100.1");

                            for (QSslCertificateExtension ext : certificate.extensions()) {
                                if (!isEv) {
                                    if (ext.oid() == "2.5.29.32") {
                                        for (QString auths : EVOids.keys()) {
                                            QString oid = EVOids.value(auths);
                                            if (ext.value().toString().contains(oid)) {
                                                //if (auths == certificateChain.at(1).issuerInfo(QSslCertificate::CommonName).first()) {
                                                    isEv = true;
                                                //}
                                            }
                                        }
                                    }
                                }
                            }
                        //}

                            if (certificate.subjectInfo(QSslCertificate::Organization).count() == 0 ||
                                    certificate.subjectInfo(QSslCertificate::CountryName).count() == 0) {
                                isEv = false;
                            }
                        if (isEv) {
                            QString organization = "";
                            QString countryName = "";
                            if (certificate.subjectInfo(QSslCertificate::Organization).count() != 0) {
                                organization = certificate.subjectInfo(QSslCertificate::Organization).first();
                            }

                            if (certificate.subjectInfo(QSslCertificate::CountryName).count() != 0) {
                                countryName = certificate.subjectInfo(QSslCertificate::CountryName).first();
                            }

                            securityMetadata.append("ev");
                            securityMetadata.append(organization + " [" + countryName + "]");
                            /*securityMetadata.append("This connection is secure, and the company has been verified by <b>" +
                                                    certificate.issuerInfo(QSslCertificate::CommonName).first() + "</b> to be <b>" +
                                                    organization + "</b>");*/
                        } else {
                            ui->securityFrame->setStyleSheet("");

                            QColor panelColor = ui->securityFrame->palette().color(QPalette::Window);
                            if (((qreal) panelColor.red() + (qreal) panelColor.green() + (qreal) panelColor.red()) / (qreal) 3 < 127) {
                                ui->securityPadlock->setPixmap(QIcon(":/icons/lock-d").pixmap(16, 16));
                            } else {
                                ui->securityPadlock->setPixmap(QIcon(":/icons/lock-l").pixmap(16, 16));
                            }

                            securityMetadata.append("secure");
                            securityMetadata.append("");
                            /*securityMetadata.append("This connection is secure, and has been verified by <b>" + certificate.issuerInfo(QSslCertificate::CommonName).first() + "</b>");*/
                        }
                        securityMetadata.append(QVariant::fromValue(certificate));

                        insertIntoMetadata(browser, "security", securityMetadata);
                        updateCurrentBrowserDisplay();
                    });
                    connect(sslSock, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
                        [=](QAbstractSocket::SocketError socketError){
                        QVariantList securityMetadata;
                        securityMetadata.append("secure");
                        securityMetadata.append("");
                        /*securityMetadata.append("This connection is secure, but details couldn't be found.");
                        insertIntoMetadata(browser, "security", securityMetadata);*/
                        updateCurrentBrowserDisplay();
                    });
                    sslSock->connectToHostEncrypted(currentUrl.host(), currentUrl.port(443));
                }
            } else {
                securityMetadata.append("unsecure");
                securityMetadata.append("");
                /*securityMetadata.append("This connection has not been encrypted.");*/

            }

            if (settings.value("data/malwareProtect", true).toBool()) {
                QNetworkAccessManager* manager = new QNetworkAccessManager;

                QNetworkRequest request(QUrl("https://safebrowsing.googleapis.com/v4/threatMatches:find?key=AIzaSyAbf9-1icT5zrHDsM0z5YSk-23lj-shvIM"));
                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
                QString requestBody = "{\n"
                                      "    \"client\": {\n"
                                      "        \"clientId\":        \"theWeb\","
                                      "        \"clientVersion\":   \"15.0\""
                                      "    },"
                                      "    \"threatInfo\": {"
                                      "        \"threatTypes\":     [\"MALWARE\", \"SOCIAL_ENGINEERING\"],"
                                      "        \"platformTypes\":   [\"ANY_PLATFORM\"],"
                                      "        \"threatEntryTypes\":[\"URL\"],"
                                      "        \"threatEntries\": ["
                                      "            {\"url\": \"" + QString::fromStdString(url.ToString()) + "\"}"
                                      "        ]"
                                      "    }"
                                      "}";
                connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply* reply) {
                    QString replyString = reply->readAll();
                    QJsonDocument JsonDoc = QJsonDocument::fromJson(replyString.toUtf8());
                    QJsonObject object = JsonDoc.object();
                    if (object.contains("matches")) { //Something was found!
                        QStringList threatMetadata;

                        QString threatType = object.value("matches").toArray().first().toObject().value("threatType").toString();
                        if (threatType == "MALWARE") {
                            threatMetadata.append("This website may contain malware. <b>We suggest that you don't visit "
                                                          "this website.</b>");
                            threatMetadata.append("Google Safe Browsing found malware on this site. Malware can cause your "
                                                        "PC to slow down or act erratically.");
                        } else if (threatType == "SOCIAL_ENGINEERING") {
                            threatMetadata.append("This website may trick you into doing something like revealing your "
                                                          "personal information (such as passwords or credit card information) or "
                                                          "installing software that you may not want. <b>We suggest that you don't visit "
                                                          "this website and enter any personal information.</b>");
                            threatMetadata.append("Google Safe Browsing found this site to be deceptive. These websites trick "
                                                        "users into doing something dangerous.");
                        }

                        insertIntoMetadata(browser, "threat", threatMetadata);
                        updateCurrentBrowserDisplay();
                    }
                });
                manager->post(request, requestBody.toUtf8());
            }
        }

        insertIntoMetadata(browser, "security", securityMetadata);
        updateCurrentBrowserDisplay();

        //Flush cookies (just to make sure)
        if (CefCookieManager::GetGlobalManager(NULL).get() != NULL) {
            CefCookieManager::GetGlobalManager(NULL).get()->FlushStore(NULL);
        }

        //Check if this is an oblivion window
        if (!isOblivion) {
            //Check if this is a theweb:// internal webpage
            if (currentUrl.scheme() != "theweb") {
                //Insert a flag into metadata of browser.
                //History item will be added during the TitleChanged event.
                insertIntoMetadata(browser, "addToHistory", true);
            }
        }
    }
}

void MainWindow::insertIntoMetadata(Browser browser, QString key, QVariant value) {
    //Add a value into a browser's metadata
    if (indexOfBrowser(browser) != -1) {
        QVariantMap currentBrowserMap = browserMetadata.at(indexOfBrowser(browser));
        currentBrowserMap.insert(key, value);
        browserMetadata.replace(indexOfBrowser(browser), currentBrowserMap);
    }
}

void MainWindow::removeFromMetadata(Browser browser, QString key) {
    //Remove a value from a browser's metadata
    QVariantMap currentBrowserMap = browserMetadata.at(indexOfBrowser(browser));

    //If the key does not exist, ignore the request.
    if (currentBrowserMap.keys().contains(key)) {
        currentBrowserMap.remove(key);
        browserMetadata.replace(indexOfBrowser(browser), currentBrowserMap);
    }
}

void MainWindow::FullscreenModeChange(Browser browser, bool fullscreen) {
    if (IsCorrectBrowser(browser)) { //Check if this is the current browser
        static bool wasMaximized = false; //Keep track of whether the window is currently maximized.

        //Check if the browser is full screen
        if (fullscreen) {
            wasMaximized = this->isMaximized();
            ui->toolBar->setVisible(false);
            tabBar->setVisible(false);
            this->showFullScreen();

            currentWarning = MainWindow::fullscreen;
            QUrl currentUrl(QString::fromStdString(browser.get()->GetMainFrame().get()->GetURL().ToString()));
            ui->warningLabel->setText(currentUrl.host() + " is now full screen. Hit [ESC] to exit full screen.");

            ui->warningOk->setVisible(true);
            ui->warningCancel->setVisible(true);
            ui->warningOk->setText("OK");
            ui->warningCancel->setText("Exit Full Screen");
            ui->warningFrame->setVisible(true);
        } else {
            ui->toolBar->setVisible(true);
            tabBar->setVisible(true);
            if (wasMaximized) {
                this->showMaximized();
            } else {
                this->showNormal();
            }

            if (currentWarning == MainWindow::fullscreen) {
                ui->warningFrame->setVisible(false);
                currentWarning = none;
            }
        }
    }
}

void MainWindow::on_warningOk_clicked()
{
    switch (currentWarning) {
    case fullscreen:
        break; //do nothing
    case notification:
    {
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("jsNotificationRequest_Reply");
        message.get()->GetArgumentList().get()->SetBool(0, true);
        browser().get()->SendProcessMessage(PID_RENDERER, message);
    }
        break;
    }
    ui->warningFrame->setVisible(false);

    currentWarning = none;
}

void MainWindow::on_warningCancel_clicked()
{
    switch (currentWarning) {
    case fullscreen:
        browser().get()->GetMainFrame().get()->ExecuteJavaScript("document.webkitExitFullscreen()", "", 0);
        break;
    case notification:
    {
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("jsNotificationRequest_Reply");
        message.get()->GetArgumentList().get()->SetBool(0, false);
        browser().get()->SendProcessMessage(PID_RENDERER, message);
    }
        break;
    }
    ui->warningFrame->setVisible(false);

    currentWarning = none;
}

void MainWindow::on_actionNew_Window_triggered()
{
    MainWindow* newWin = new MainWindow();
    newWin->show();
}

void MainWindow::JSDialog(CefRefPtr<CefBrowser> browser, const CefString &origin_url, CefHandler::JSDialogType dialog_type, const CefString &message_text, const CefString &default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool &suppress_message) {
    if (indexOfBrowser(browser) != -1) {
        QVariantList JsMetadata;

        switch (dialog_type) {
        case JSDIALOGTYPE_ALERT:
            JsMetadata.append("alert");
            break;
        case JSDIALOGTYPE_CONFIRM:
            JsMetadata.append("confirm");
            break;
        case JSDIALOGTYPE_PROMPT:
            JsMetadata.append("prompt");
            break;
        }

        JsMetadata.append(QString::fromStdString(message_text.ToString()));
        JsMetadata.append(QVariant::fromValue(callback));
        JsMetadata.append(QString::fromStdString(default_prompt_text));

        insertIntoMetadata(browser, "js", JsMetadata);

        updateCurrentBrowserDisplay();
    }
}

void MainWindow::LoadingStateChange(Browser browser, bool isLoading, bool canGoBack, bool canGoForward) {
    if (indexOfBrowser(browser) != -1) {
        if (isLoading) {
            ui->errorFrame->setVisible(false);
            removeFromMetadata(browser, "error");
            removeFromMetadata(browser, "crash");
        } else {
            tabBar->setTabIcon(indexOfBrowser(browser), browserIcons.at(indexOfBrowser(browser)));
        }

        updateCurrentBrowserDisplay();
    }
}

void MainWindow::LoadError(Browser browser, CefRefPtr<CefFrame> frame, CefHandler::ErrorCode errorCode, const CefString &errorText, const CefString &failedUrl) {
    if (indexOfBrowser(browser) != -1) {
        if (errorCode != ERR_ABORTED && errorCode != ERR_UNKNOWN_URL_SCHEME && frame.get()->IsMain()) {
            QStringList errorDisplayMetadata;

            switch (qrand() % 7) {
            case 0:
                errorDisplayMetadata.append("This isn't supposed to happen...");
                break;
            case 1:
                errorDisplayMetadata.append("Can't connect to webpage");
                break;
            case 2:
                errorDisplayMetadata.append("Well...");
                break;
            case 3:
                errorDisplayMetadata.append("Well, that didn't work.");
                break;
            case 4:
                errorDisplayMetadata.append("Something's not right.");
                break;
            case 5:
                errorDisplayMetadata.append("Whoa!");
                break;
            case 6:
                errorDisplayMetadata.append("Oops!");
                break;
            }


            switch (errorCode) {
            case ERR_NAME_NOT_RESOLVED:
                errorDisplayMetadata.append("Couldn't find server");
                break;
            case ERR_TIMED_OUT:
                errorDisplayMetadata.append("Server took too long to respond");
                break;
            case ERR_ACCESS_DENIED:
                errorDisplayMetadata.append("Access Denied");
                break;
            case ERR_CACHE_MISS:
                errorDisplayMetadata.replace(0, "Confirm Form Resubmission");
                errorDisplayMetadata.append("To display this webpage, data that you entered previously needs to be sent again. To do so, "
                                       "click \"Reload.\" However, note that by doing this, you will repeat any action that this page "
                                       "performed.");
                break;
            case ERR_CONNECTION_RESET:
                errorDisplayMetadata.append("The connection was reset");
                break;
            case ERR_CONNECTION_REFUSED:
                errorDisplayMetadata.append("The server refused the connection");
                break;
            case ERR_CONNECTION_CLOSED:
            case ERR_EMPTY_RESPONSE:
                errorDisplayMetadata.append("The server didn't send anything");
                break;
            case ERR_INTERNET_DISCONNECTED:
                errorDisplayMetadata.append("Disconnected from the internet");
                break;
            case ERR_TOO_MANY_REDIRECTS:
                errorDisplayMetadata.append("A redirect loop was detected");
                break;
            case ERR_UNSAFE_REDIRECT:
                errorDisplayMetadata.append("Redirect not allowed");
                break;
            case ERR_UNSAFE_PORT:
                errorDisplayMetadata.append("Disallowed Port");
                break;
            case ERR_INVALID_RESPONSE:
                errorDisplayMetadata.append("Invalid Response");
                break;
            case ERR_INVALID_URL:
                errorDisplayMetadata.append("Invalid URL");
                break;
            case -21: //Network change detected
                errorDisplayMetadata.append("The internet connection changed while we were trying to connect to the page.");
                break;
            //SSL loading errors:
            case ERR_SSL_PROTOCOL_ERROR:
                errorDisplayMetadata.replace(0, "This webpage is having trouble providing a secure connection");
                errorDisplayMetadata.append("This website sent us something that theWeb didn't understand.");
            case ERR_SSL_CLIENT_AUTH_CERT_NEEDED:
                errorDisplayMetadata.replace(0, "theWeb doesn't support this website");
                errorDisplayMetadata.append("This website is asking for a client certificate, which theWeb doesn't support (yet.)");
            default:
                errorDisplayMetadata.append(QString::fromStdString(failedUrl.ToString()) + " may be down or it may have moved somewhere else. " );
            }

            insertIntoMetadata(browser, "error", errorDisplayMetadata);


            if (settings.value("browser/tabText", false).toBool()) {
                tabBar->setTabText(indexOfBrowser(browser), tabBar->fontMetrics().elidedText(errorDisplayMetadata.at(0), Qt::ElideRight, 200));
            } else {
                tabBar->setTabText(indexOfBrowser(browser), "");
            }
            tabBar->setTabData(indexOfBrowser(browser), errorDisplayMetadata.at(0));
            //tabBar->setTabToolTip(indexOfBrowser(browser), errorDisplayMetadata.at(0));

            QIcon icon = QIcon::fromTheme("dialog-error");
            tabBar->setTabIcon(indexOfBrowser(browser), QIcon(icon));
            browserIcons.replace(indexOfBrowser(browser), icon);

            updateCurrentBrowserDisplay();
        }
    }
}

void MainWindow::BeforeClose(Browser browser) {
    if (indexOfBrowser(browser) != -1) {
        //Flush cookies (to make sure)
        if (CefCookieManager::GetGlobalManager(NULL).get() != NULL) {
            CefCookieManager::GetGlobalManager(NULL).get()->FlushStore(NULL);
        }

        //Actually close the tab
        int index = indexOfBrowser(browser);
        ui->browserStack->removeWidget(ui->browserStack->widget(index));
        browserMetadata.removeAt(index);
        browserIcons.removeAt(index);
        browserList.removeAt(index);
        tabBar->removeTab(index);
        if (tabBar->count() == 0) {
            this->close();
        }
        updateCurrentBrowserDisplay();
    }
}

void MainWindow::on_JsDialogOk_clicked()
{
    ui->JsDialogFrame->setVisible(false);
    ui->browserStack->setVisible(true);
    QVariantMap metadata = browserMetadata.at(indexOfBrowser(browser()));
    if (metadata.keys().contains("protocol")) {
        QProcess::startDetached("xdg-open \"" + metadata.value("protocol").toString() + "\"");
        removeFromMetadata(browser(), "protocol");
        updateCurrentBrowserDisplay();
    } else {
        browserMetadata.at(indexOfBrowser(browser())).value("js").toList().at(2).value<CefRefPtr<CefJSDialogCallback>>().get()->Continue(true, ui->JsDialogPrompt->text().toStdString());
        removeFromMetadata(browser(), "js");
        updateCurrentBrowserDisplay();
    }
}

void MainWindow::on_JsDialogCancel_clicked()
{
    QVariantMap metadata = browserMetadata.at(indexOfBrowser(browser()));
    if (metadata.keys().contains("protocol")) {
        removeFromMetadata(browser(), "protocol");
        updateCurrentBrowserDisplay();
    } else {
        ui->JsDialogFrame->setVisible(false);
        ui->browserStack->setVisible(true);
        browserMetadata.at(indexOfBrowser(browser())).value("js").toList().at(2).value<CefRefPtr<CefJSDialogCallback>>().get()->Continue(false, "");
        removeFromMetadata(browser(), "js");
        updateCurrentBrowserDisplay();
    }
}

void MainWindow::BeforeUnloadDialog(Browser browser, const CefString &message_text, bool is_reload, CefRefPtr<CefJSDialogCallback> callback) {
    if (indexOfBrowser(browser) != -1) {
        QVariantList JsMetadata;

        JsMetadata.append("unload");
        JsMetadata.append("If you've made some changes in this website, they might not be saved.");
        JsMetadata.append(QVariant::fromValue(callback));
        JsMetadata.append(is_reload);

        insertIntoMetadata(browser, "js", JsMetadata);
        updateCurrentBrowserDisplay();
    }
}

void MainWindow::on_JsDialogPrompt_returnPressed()
{
    ui->JsDialogOk->click();
}

void MainWindow::AuthCredentials(Browser browser, CefRefPtr<CefFrame> frame, bool isProxy, const CefString &host, int port, const CefString &realm, const CefString &scheme, CefRefPtr<CefAuthCallback> callback) {
    if (IsCorrectBrowser(browser)) {
        QVariantList AuthMetadata;
        AuthMetadata.append("Log in to " + QString::fromStdString(host.ToString()) + ":" + QString::number(port));
        AuthMetadata.append("Server Realm: " + QString::fromStdString(realm.ToString()));
        AuthMetadata.append(QUrl(QString::fromStdString(frame.get()->GetURL().ToString())).scheme() == "http" && scheme == "basic");
        AuthMetadata.append(QVariant::fromValue(callback));
        insertIntoMetadata(browser, "auth", AuthMetadata);
        updateCurrentBrowserDisplay();
    }
}

void MainWindow::on_AuthOk_clicked()
{
    ui->AuthFrame->setVisible(false);
    ui->browserStack->setVisible(true);
    browserMetadata.at(indexOfBrowser(browser())).value("auth").toList().at(3).value<CefRefPtr<CefAuthCallback>>().get()->Continue(ui->AuthUsername->text().toStdString(), ui->AuthPassword->text().toStdString());
    removeFromMetadata(browser(), "auth");
}

void MainWindow::on_AuthCancel_clicked()
{
    ui->AuthFrame->setVisible(false);
    ui->browserStack->setVisible(true);
    browserMetadata.at(indexOfBrowser(browser())).value("auth").toList().at(3).value<CefRefPtr<CefAuthCallback>>().get()->Cancel();
    removeFromMetadata(browser(), "auth");
}

void MainWindow::on_AuthUsername_returnPressed()
{
    ui->AuthOk->click();
}

void MainWindow::on_AuthPassword_returnPressed()
{
    ui->AuthOk->click();
}

void MainWindow::BeforePopup(Browser browser, CefRefPtr<CefFrame> frame, const CefString &target_url, const CefString &target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures &popupFeatures, CefWindowInfo *windowInfo, CefBrowserSettings settings, bool *no_javascript_access) {
    if (indexOfBrowser(browser) != -1) {
        /*if (!user_gesture) {
            if (QMessageBox::information(this, "Open pop-up?", "This webpage is requesting that " + QString::fromStdString(target_url.ToString()) + " be opened in a new window. Is that OK?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
                return;
            }
        }

        Browser newBrowser;
        if (isOblivion) {
            CefBrowserSettings settings = defaultBrowserSettings;
            settings.application_cache = STATE_DISABLED;

            CefRequestContextSettings contextSettings;
            CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(contextSettings, new OblivionRequestContextHandler);
            context.get()->RegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
            newBrowser = browser.get()->GetHost().get()->CreateBrowserSync(*windowInfo, CefRefPtr<CefHandler>(handler), target_url, settings, context);
        } else {
            newBrowser = browser.get()->GetHost().get()->CreateBrowserSync(*windowInfo, CefRefPtr<CefHandler>(handler), target_url, defaultBrowserSettings, CefRefPtr<CefRequestContext>());
        }

        switch (target_disposition) {
        case WOD_NEW_FOREGROUND_TAB:
            createNewTab(newBrowser);
            break;
        case WOD_NEW_BACKGROUND_TAB:
            createNewTab(newBrowser, true);
            break;
        case WOD_NEW_WINDOW:
        {
            MainWindow* w = new MainWindow(browser);
            w->show();
        }
            break;
        }
*/

        if (target_disposition == WOD_NEW_BACKGROUND_TAB || target_disposition == WOD_NEW_FOREGROUND_TAB || target_disposition == WOD_NEW_POPUP || target_disposition == WOD_SINGLETON_TAB) {
            handler->setNewBrowserTabWindow(this);
        }
    }
}

void MainWindow::on_actionLimit_to_60_fps_triggered()
{
    batteryCheckTimer.stop();
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(true);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(1000 / 60);
    emit signalBroker->ReloadSettings();
}

void MainWindow::on_actionLimit_to_30_fps_triggered()
{
    batteryCheckTimer.stop();
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(true);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(1000 / 30);
    emit signalBroker->ReloadSettings();
}

void MainWindow::on_actionLimit_to_15_fps_triggered()
{
    batteryCheckTimer.stop();
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(true);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(1000 / 15);
    emit signalBroker->ReloadSettings();
}

void MainWindow::on_actionLimit_to_1_fps_triggered()
{
    batteryCheckTimer.stop();
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(true);
    cefEventLoopTimer.setInterval(1000);
    emit signalBroker->ReloadSettings();
}

void MainWindow::on_actionDon_t_Limit_triggered()
{
    batteryCheckTimer.stop();
    ui->actionDon_t_Limit->setChecked(true);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(0);
    emit signalBroker->ReloadSettings();
}

void MainWindow::on_actionAbout_theWeb_triggered()
{
    if (QString::fromStdString(browser().get()->GetMainFrame().get()->GetURL().ToString()) == settings.value("browser/home", "theweb://newtab").toString()) {
        browser().get()->GetMainFrame().get()->LoadURL("theweb://theweb");
    } else {
        CefWindowInfo windowInfo;
        CefBrowserSettings settings = defaultBrowserSettings;

        Browser browser = CefBrowserHost::CreateBrowserSync(windowInfo, handler, "theweb://theweb", settings, CefRefPtr<CefRequestContext>());

        createNewTab(browser);
    }
}

void MainWindow::on_actionSettings_triggered()
{
    if (QString::fromStdString(browser().get()->GetMainFrame().get()->GetURL().ToString()) == settings.value("browser/home", "theweb://newtab").toString()) {
        browser().get()->GetMainFrame().get()->LoadURL("theweb://settings");
    } else {
        CefWindowInfo windowInfo;
        CefBrowserSettings settings = defaultBrowserSettings;

        Browser browser = CefBrowserHost::CreateBrowserSync(windowInfo, handler, "theweb://settings", settings, CefRefPtr<CefRequestContext>());

        createNewTab(browser);
    }
}

void MainWindow::on_fraudIgnore_clicked()
{
    if (!ui->fraudExtraFrame->isVisible()) {
        ui->fraudExtraFrame->setVisible(true);
        ui->fraudIgnore->setText("Continue anyway");
    } else {
        ui->fraudFrame->setVisible(false);
        ui->fraudIgnore->setText("More Info");
        ui->fraudExtraFrame->setVisible(false);
        ui->browserStack->setVisible(true);
        removeFromMetadata(browser(), "threat");
    }
}

void MainWindow::on_fraudBack_clicked()
{
    if (browser().get()->CanGoBack()) {
        ui->actionGo_Back->trigger();
    } else {
        browser().get()->GetHost().get()->CloseBrowser(true);
    }
    ui->fraudFrame->setVisible(false);
    ui->browserStack->setVisible(true);
    removeFromMetadata(browser(), "threat");
}

void MainWindow::on_securityFrame_clicked()
{
    /*if (ui->securityInfoFrame->isVisible()) {
        ui->securityInfoFrame->setVisible(false);
    } else {
        ui->securityInfoFrame->setVisible(true);
    }*/

    CertificateInfo* infoWindow;
    QString type = browserMetadata.at(indexOfBrowser(browser())).value("security").toList().at(0).toString();
    if (browserMetadata.at(indexOfBrowser(browser())).value("security").toList().count() > 2) {
        infoWindow = new CertificateInfo(type, browserMetadata.at(indexOfBrowser(browser())).value("security").toList().at(2).value<QSslCertificate>(), this);
    } else {
        infoWindow = new CertificateInfo(type, this);
    }
    infoWindow->exec();
    infoWindow->deleteLater();
}

void MainWindow::CertificateError(Browser browser, cef_errorcode_t cert_error, const CefString &request_url, CefRefPtr<CefSSLInfo> ssl_info, CefRefPtr<CefRequestCallback> callback) {
    if (indexOfBrowser(browser) != -1) {
        QVariantList certErrorMetadata;

        switch (cert_error) {
        case ERR_CERT_COMMON_NAME_INVALID:
        {
            QString name;
            //CEF seems to be having some issues.
            //if (ssl_info.get() != NULL && ssl_info.get()->GetSubject().get() != NULL) {
                //name = QString::fromStdString(ssl_info.get()->GetSubject().get()->GetDisplayName().ToString());
            //} else {
                name = "another server.";
            //}
            certErrorMetadata.append("You're trying to connect to <b>" + QUrl(QString::fromStdString(request_url.ToString())).host() + "</b> but the server presented itself as <b>" +
                                      name + "</b>.");
            break;
        }
        case ERR_CERT_DATE_INVALID:
            certErrorMetadata.append("The server presented a certificate that either seems to not be valid yet, or has expired. <b>Check your system clock "
                                      "and if it is incorrect, set it to the correct time.</b>");
            break;
        case ERR_CERT_AUTHORITY_INVALID:
            certErrorMetadata.append("The server presented a certificate which was signed by an authority that theWeb doesn't know about. This could mean that "
                                      "an attacker could be intercepting your connection and providing his own certificate, or it could mean that theWeb "
                                      "doesn't have this certificate authority in the database.");
            break;
        case ERR_CERT_CONTAINS_ERRORS:
        case ERR_CERT_INVALID:
            certErrorMetadata.append("The server tried to tell theWeb who it was, but theWeb received something that didn't make sense.");
            break;
        case ERR_CERT_UNABLE_TO_CHECK_REVOCATION:
            certErrorMetadata.append("The server sent us a certificate, but theWeb couldn't tell whether it has been revoked or not.");
            break;
        case ERR_CERT_REVOKED:
            certErrorMetadata.append("The server sent us a certificate that has been revoked.");
            break;
        case ERR_CERT_NON_UNIQUE_NAME:
            certErrorMetadata.append("The server sent us a certificate for a non-unique hostname.");
            break;
        default:
            certErrorMetadata.append("The server sent us a certificate that theWeb doesn't trust.");
            break;
        }

        QUrl url(QString::fromStdString(request_url.ToString()));
        bool isHsts = false;
        //Check if this is a HSTS website
        QSettings hstsSettings(QApplication::organizationName(), "theWeb.hsts");
        for (QString group : hstsSettings.childGroups()) {
            hstsSettings.beginGroup(group);
            if (hstsSettings.value("Expiry").toLongLong() < QDateTime::currentMSecsSinceEpoch()) {
                //This one has expired. Remove it.
                hstsSettings.remove("");
            } else {
                if (hstsSettings.value("includeSubDomains").toBool()) {
                    if (url.topLevelDomain() == group || url.host() == group) {
                        isHsts = true;
                    }
                } else {
                    if (group == url.host()) {
                        isHsts = true;
                    }
                }
            }
            hstsSettings.endGroup();
        }

        //certCallback = callback;
        certErrorMetadata.append(QVariant::fromValue(callback));
        certErrorMetadata.append(isHsts);
        if (isHsts) {
            certErrorMetadata.append("This website has requested theWeb to not connect to it over an insecure connection.");
        } else {
            certErrorMetadata.append("If you want to continue despite this warning, click the \"Continue Anyway\" button above. Legitamate banks and businesses won't tell you to do this.");
        }

        certErrorUrls.append(QString::fromStdString(request_url.ToString()));

        QVariantList securityMetadata;
        securityMetadata.append("certerr");
        securityMetadata.append("Insecure Connection");
        if (browserMetadata.at(indexOfBrowser(browser)).value("security").toList().count() > 2) {
            securityMetadata.append(browserMetadata.at(indexOfBrowser(browser)).value("security").toList().at(2));
        }
        //securityMetadata.append("This connection may have been intercepted.");
        insertIntoMetadata(browser, "security", securityMetadata);

        insertIntoMetadata(browser, "certificate", certErrorMetadata);

        updateCurrentBrowserDisplay();
    }
}

void MainWindow::on_certIgnore_clicked()
{
    if (ui->certMoreInfo->isVisible()) {
        ui->certMoreInfo->setVisible(false);
        ui->badCertificateFrame->setVisible(false);
        ui->certIgnore->setText("More Info");
        ui->browserStack->setVisible(true);

        browserMetadata.at(indexOfBrowser(browser())).value("certificate").toList().at(1).value<CefRefPtr<CefRequestCallback>>().get()->Continue(true);
        removeFromMetadata(browser(), "certificate");
    } else {
        ui->certMoreInfo->setVisible(true);
        if (browserMetadata.at(indexOfBrowser(browser())).value("certificate").toList().at(2).toBool()) {
            //This is a HSTS website, and the user is not allowed to continue to it.
            ui->certIgnore->setVisible(false);
        } else {
            ui->certIgnore->setText("Continue Anyway");
        }
    }
}

void MainWindow::on_certificateBack_clicked()
{
    ui->actionGo_Back->trigger();
    browserMetadata.at(indexOfBrowser(browser())).value("certificate").toList().at(1).value<CefRefPtr<CefRequestCallback>>().get()->Continue(false);
    removeFromMetadata(browser(), "certificate");
    ui->badCertificateFrame->setVisible(false);
    ui->browserStack->setVisible(true);
}

void MainWindow::ReloadSettings() {
    if (settings.value("browser/toolbarOnBottom").toBool()) {
        this->addToolBar(Qt::BottomToolBarArea, ui->toolBar);
        tabBar->setShape(QTabBar::RoundedSouth);
        ((QBoxLayout*) ui->centralwidget->layout())->addWidget(tabBar);
    } else {
        this->addToolBar(Qt::TopToolBarArea, ui->toolBar);
        tabBar->setShape(QTabBar::RoundedNorth);
        ((QBoxLayout*) ui->centralwidget->layout())->insertWidget(ui->centralwidget->layout()->indexOf(ui->warningFrame), tabBar);
    }

    for (int i = 0; i < tabBar->count(); i++) {
        if (settings.value("browser/tabText", false).toBool()) {
            tabBar->setTabText(i, tabBar->fontMetrics().elidedText(tabBar->tabData(i).toString(), Qt::ElideRight, 200));
        } else {
            tabBar->setTabText(i, "");
        }
    }

    if (settings.value("browser/tabText", false).toBool()) {
        tabBar->setStyleSheet("QTabBar::tab { height: 32px; width: 260px; }");
    } else {
        tabBar->setStyleSheet("QTabBar::tab { height: 32px; }"); //Automatic width
    }

    {
        //Update the FPS limit checkbox
        ui->actionDon_t_Limit->setChecked(false);
        ui->actionLimit_to_60_fps->setChecked(false);
        ui->actionLimit_to_30_fps->setChecked(false);
        ui->actionLimit_to_15_fps->setChecked(false);
        ui->actionLimit_to_1_fps->setChecked(false);

        switch (cefEventLoopTimer.interval()) {
        case 0:
            ui->actionDon_t_Limit->setChecked(true);
            break;
        case (1000 / 60):
            ui->actionLimit_to_60_fps->setChecked(true);
            break;
        case (1000 / 30):
            ui->actionLimit_to_30_fps->setChecked(true);
            break;
        case (1000 / 15):
            ui->actionLimit_to_15_fps->setChecked(true);
            break;
        case 1000:
            ui->actionLimit_to_1_fps->setChecked(true);
            break;
        }
    }
}

void MainWindow::FaviconURLChange(Browser browser, std::vector<CefString> urls) {
    if (indexOfBrowser(browser) != -1) {
        int count = 0;
        for (CefString url : urls) {
            browser.get()->GetHost().get()->DownloadImage(url, true, 16, false, new DownloadImageCallback([=](CefRefPtr<CefImage> image) {
                if (indexOfBrowser(browser) != -1) {
                    if (image.get() == NULL) {
                        browserIcons.replace(indexOfBrowser(browser), QIcon::fromTheme("text-html"));
                    } else {
                        int width = 16, height = 16;

                        CefRefPtr<CefBinaryValue> binary = image.get()->GetAsPNG(1, true, width, height);

                        unsigned char* data = (unsigned char *) malloc(32767);
                        uint read = binary.get()->GetData(data, 32767, 0);
                        QPixmap pixmap;
                        pixmap.loadFromData(data, read);

                        if (!pixmap.isNull()) {
                            QIcon icon(pixmap);
                            tabBar->setTabIcon(indexOfBrowser(browser), QIcon(icon));
                            browserIcons.replace(indexOfBrowser(browser), icon);
                        }

                        free(data);
                    }
                }
            }));
            count++;
        }

        if (count == 0) {
            browserIcons.replace(indexOfBrowser(browser), QIcon::fromTheme("text-html"));
        }
    }
}

Browser MainWindow::browser() {
    if (tabBar->currentIndex() < browserList.count() && tabBar->currentIndex() != -1) {
        return browserList.at(tabBar->currentIndex());
    } else {
        return NULL;
    }
}

Browser MainWindow::getBrowserFor(Browser browser) {
    for (Browser br : browserList) {
        if (br.get()->IsSame(browser)) {
            return br;
        }
    }
}

int MainWindow::indexOfBrowser(Browser browser) {
    for (int i = 0; i < browserList.count(); i++) {
        if (browserList.at(i).get()->IsSame(browser)) {
            return i;
        }
    }
    return -1;
}

void MainWindow::updateCurrentBrowserDisplay() {
    if (tabBar->currentIndex() != -1) {
        bool showBrowserStack = true;
        bool enableTabBar = true;

        ui->spaceSearch->setCurrentUrl(QUrl(QString::fromStdString(browser().get()->GetMainFrame().get()->GetURL())));
        this->setWindowTitle(tabBar->tabData(tabBar->currentIndex()).toString().append(" - theWeb"));
        ui->actionGo_Back->setEnabled(browser().get()->CanGoBack());
        ui->actionGo_Forward->setEnabled(browser().get()->CanGoForward());

        QVariantMap metadata = browserMetadata.at(indexOfBrowser(browser()));

        QStringList securityMetadata = metadata.value("security").toStringList();
        if (securityMetadata.count() != 0) {
            if (securityMetadata.at(0) == "theweb") {
                ui->securityFrame->setStyleSheet("");
                ui->securityPadlock->setPixmap(QIcon(":/icons/icon").pixmap(16, 16));
            } else if (securityMetadata.at(0) == "certerr") {
                ui->securityFrame->setStyleSheet("background-color: #640000; color: white;");
                ui->securityPadlock->setPixmap(QIcon(":/icons/badsecure").pixmap(16, 16));
            } else if (securityMetadata.at(0) == "ev") {
                ui->securityFrame->setStyleSheet("background-color: #006400; color: white;");
                ui->securityPadlock->setPixmap(QIcon(":/icons/lock-d").pixmap(16, 16));
            } else if (securityMetadata.at(0) == "secure") {
                ui->securityFrame->setStyleSheet("");

                QColor panelColor = ui->securityFrame->palette().color(QPalette::Window);
                if (((qreal) panelColor.red() + (qreal) panelColor.green() + (qreal) panelColor.red()) / (qreal) 3 < 127) {
                    ui->securityPadlock->setPixmap(QIcon(":/icons/lock-d").pixmap(16, 16));
                } else {
                    ui->securityPadlock->setPixmap(QIcon(":/icons/lock-l").pixmap(16, 16));
                }
            } else {
                ui->securityPadlock->setPixmap(QIcon::fromTheme("text-html").pixmap(16, 16));
                ui->securityFrame->setStyleSheet("");
            }

            if (securityMetadata.at(1) == "") {
                ui->securityEVName->setVisible(false);
            } else {
                ui->securityEVName->setText(securityMetadata.at(1));
                ui->securityEVName->setVisible(true);
            }

            //ui->securityText->setText(securityMetadata.at(2));
        }

        if (metadata.keys().contains("error") || metadata.keys().contains("crash")) {
            ui->errorFrame->setVisible(true);
            if (metadata.keys().contains("crash")) {
                QStringList crashMetadata = metadata.value("crash").toStringList();
                ui->errorTitle->setText(crashMetadata.at(0));
                ui->errorText->setText(crashMetadata.at(1));
            } else {
                QStringList errorMetadata = metadata.value("error").toStringList();
                ui->errorTitle->setText(errorMetadata.at(0));
                ui->errorText->setText(errorMetadata.at(1));
            }
            showBrowserStack = false;
        } else {
            ui->errorFrame->setVisible(false);
        }

        if (metadata.keys().contains("protocol")) {
            QString url = metadata.value("protocol").toString();

            ui->JsBeforeLeaveTitle->setText("Open External Program");
            ui->JsDialogText->setText("Do you want to open an external program to open " + url);
            ui->JsDialogOk->setText("Open Program");
            ui->JsDialogCancel->setText("Do Nothing");
            ui->JsDialogPrompt->setVisible(false);
            ui->JsDialogFrame->setVisible(true);
            showBrowserStack = false;
        } else if (metadata.keys().contains("js")) {
            QVariantList JsMetadata = metadata.value("js").toList();

            if (JsMetadata.at(0).toString() == "unload") {
                ui->JsDialogText->setText(JsMetadata.at(1).toString());
                ui->JsDialogFrame->setVisible(true);
                ui->JsDialogCancel->setVisible(true);
                if (JsMetadata.at(3).toBool()) {
                    ui->JsDialogOk->setText("Reload anyway");
                    ui->JsDialogCancel->setText("Don't Reload");
                } else {
                    ui->JsDialogOk->setText("Leave anyway");
                    ui->JsDialogCancel->setText("Don't leave");
                }
                ui->JsBeforeLeaveTitle->setText("Hold up!");
                ui->JsBeforeLeaveTitle->setVisible(true);
                ui->JsDialogPrompt->setVisible(false);
            } else {
                ui->JsDialogFrame->setVisible(true);
                if (QUrl(QString::fromStdString(browser().get()->GetMainFrame().get()->GetURL().ToString())).scheme() == "theweb") {
                    QStringList splitText = JsMetadata.at(1).toString().split(":");
                    ui->JsDialogOk->setText(splitText.at(2));
                    ui->JsDialogCancel->setText(splitText.at(3));
                    ui->JsBeforeLeaveTitle->setText(splitText.at(0));
                    ui->JsBeforeLeaveTitle->setVisible(true);
                    ui->JsDialogText->setText(splitText.at(1));
                } else {
                    ui->JsDialogOk->setText("OK");
                    ui->JsDialogCancel->setText("Cancel");
                    ui->JsBeforeLeaveTitle->setVisible(false);
                    ui->JsDialogText->setText(JsMetadata.at(1).toString());
                }

                if (JsMetadata.at(0) == "alert") {
                    ui->JsDialogCancel->setVisible(false);
                    ui->JsDialogPrompt->setVisible(false);
                } else if (JsMetadata.at(0) == "confirm") {
                    ui->JsDialogCancel->setVisible(true);
                    ui->JsDialogPrompt->setVisible(false);
                } else {
                    ui->JsDialogCancel->setVisible(true);
                    ui->JsDialogPrompt->setText(JsMetadata.at(3).toString());
                    ui->JsDialogPrompt->setVisible(true);

                    ui->JsDialogPrompt->setFocus();
                }
            }
            showBrowserStack = false;
        } else {
            ui->JsDialogFrame->setVisible(false);
            ui->JsDialogPrompt->setText("");
        }

        if (metadata.keys().contains("certificate")) {
            QVariantList certErrorMetadata = metadata.value("certificate").toList();
            ui->certMoreText->setText(certErrorMetadata.at(0).toString());
            ui->certContinueWarning->setText(certErrorMetadata.at(3).toString());
            if (!certErrorMetadata.at(2).toBool()) {
                //There is no HSTS. Show the ignore button.
                ui->certIgnore->setVisible(true);
            }

            ui->badCertificateFrame->setVisible(true);
            showBrowserStack = false;
        } else {
            ui->badCertificateFrame->setVisible(false);
            ui->certMoreInfo->setVisible(false);
            ui->certIgnore->setText("More Info");
            ui->certIgnore->setVisible(true);
        }

        if (metadata.keys().contains("auth")) {
            QVariantList AuthMetadata = metadata.value("auth").toList();
            ui->AuthHost->setText(AuthMetadata.at(0).toString());
            ui->AuthRealm->setText(AuthMetadata.at(1).toString());
            ui->AuthUsername->setText("");
            ui->AuthPassword->setText("");

            if (AuthMetadata.at(2).toBool()) {
                ui->AuthBASIC->setVisible(true);
            } else {
                ui->AuthBASIC->setVisible(false);
            }

            ui->AuthFrame->setVisible(true);
            showBrowserStack = false;
        } else {
            ui->AuthFrame->setVisible(false);
        }

        if (metadata.keys().contains("threat")) {
            QStringList threatMetadata = metadata.value("threat").toStringList();

            ui->fraudExplanation->setText(threatMetadata.at(0));
            ui->fraudExtraText->setText(threatMetadata.at(1));

            ui->fraudFrame->setVisible(true);
            showBrowserStack = false;

            //Hide the JS dialog frame in case website is doing something bad
            ui->JsDialogFrame->setVisible(false);
        } else {
            ui->fraudFrame->setVisible(false);
        }

        if (metadata.keys().contains("filePicker")) {
            showBrowserStack = false;
            ui->SelectFileFrame->setVisible(true);
            enableTabBar = false;
        } else {
            ui->SelectFileFrame->setVisible(false);
        }

        if (metadata.keys().contains("print")) {
            showBrowserStack = false;
            ui->printFrame->setVisible(true);
            enableTabBar = false;
        } else {
            ui->printFrame->setVisible(false);
        }

        ui->browserStack->setVisible(showBrowserStack);
        tabBar->setEnabled(enableTabBar);
    }
}

void MainWindow::on_spaceSearch_GotFocus()
{
    if (ui->browserStack->isVisible()) {
        ui->browserStack->setVisible(false);
        ui->browserStack->setVisible(true);
    }
}

void MainWindow::KeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent &event, XEvent *os_event) {
    if (indexOfBrowser(browser) != -1 && os_event) {
        if (os_event->xkey.type == 2) { //KeyPress
            if (os_event->xkey.state == ControlMask) {
                if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_T)) { //New Tab
                    if (tabBar->isVisible()) ui->actionNew_Tab->trigger();
                } else if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_N)) { //New Window
                    ui->actionNew_Window->trigger();
                } else if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_W)) { //Close Tab
                    ui->actionClose_Tab->trigger();
                } else if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_comma)) { //Settings
                    ui->actionSettings->trigger();
                } else if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_F)) { //Find
                    ui->actionFind->trigger();
                } else if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_H)) { //History
                    ui->actionHistory->trigger();
                } else if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_P)) { //Print
                    ui->actionPrint->trigger();
                }
            } else if (os_event->xkey.state == (ControlMask | ShiftMask)) {
                if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_N)) { //New Oblivion Window
                    ui->actionNew_Oblivion_Window->trigger();
                }
            }
        }
    }
}

void MainWindow::on_actionClose_Tab_triggered()
{
    browserList.at(tabBar->currentIndex()).get()->GetHost().get()->CloseBrowser(false);
}

void MainWindow::on_actionNew_Oblivion_Window_triggered()
{
    MainWindow* window = new MainWindow(NULL, true);
    window->show();
}

void MainWindow::ContextMenu(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback) {
    if (IsCorrectBrowser(browser)) {
        QMenu* menu = new QMenu();
        bool selectedItem = false;
        for (int i = 0; i < model.get()->GetCount(); i++) {
            int commandId = model.get()->GetCommandIdAt(i);
            switch (model.get()->GetType(commandId)) {
            case MENUITEMTYPE_COMMAND:
            {
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
                case CefHandler::CopyLink:
                    icon = "edit-copy";
                    break;
                case CefHandler::OpenLinkInNewTab:
                    icon = "tab-new";
                    break;
                case CefHandler::OpenLinkInNewWindow:
                case CefHandler::OpenLinkInNewOblivion:
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
            }
                break;
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
        //menu->exec(ui->browserStack->mapToGlobal(QPoint(params.get()->GetXCoord(), params.get()->GetYCoord())));
        menu->popup(ui->browserStack->mapToGlobal(QPoint(params.get()->GetXCoord(), params.get()->GetYCoord())));
        //delete menu;
    }
}

void MainWindow::ContextMenuCommand(Browser browser, int command_id, CefRefPtr<CefContextMenuParams> params) {
    if (indexOfBrowser(browser) != -1) {
        switch (command_id) {
        case CefHandler::OpenLinkInNewTab:
        {
            Browser newBrowser;
            if (isOblivion) {
                CefBrowserSettings settings = defaultBrowserSettings;
                settings.application_cache = STATE_DISABLED;

                CefRequestContextSettings contextSettings;
                CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(contextSettings, new OblivionRequestContextHandler);
                context.get()->RegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
                newBrowser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, params.get()->GetLinkUrl(), settings, context);
            } else {
                newBrowser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, params.get()->GetLinkUrl(), defaultBrowserSettings, CefRefPtr<CefRequestContext>());
            }
            createNewTab(newBrowser);
        }
            break;
        case CefHandler::DevTools:
        {
            browser.get()->GetHost().get()->ShowDevTools(CefWindowInfo(), handler, defaultBrowserSettings, CefPoint(params.get()->GetXCoord(), params.get()->GetYCoord()));
            handler->newWindowIsDevToolsWindow = false;
        }
            break;
        }

    }
}

void MainWindow::ProtocolExecution(Browser browser, const CefString &url, bool &allow_os_execution) {
    insertIntoMetadata(browser, "protocol", QString::fromStdString(url.ToString()));
}

void MainWindow::Tooltip(Browser browser, CefString &text) {
    //Check if this is the current browser tab
    if (IsCorrectBrowser(browser)) {
        //Show the tooltip
        QToolTip::showText(QCursor::pos(), QString::fromStdString(text.ToString()));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    ui->hoverUrlLabel->raise();
    ui->browserStack->lower();
    ui->hoverUrlLabel->move(10, this->height() - ui->hoverUrlLabel->height() - 10);
}

void MainWindow::ShowBrowser(Browser browser) {
    if (indexOfBrowser(browser) != -1) {
        this->show();
        this->raise();
        ui->browserStack->setCurrentIndex(indexOfBrowser(browser));
    }
}

void MainWindow::AskForNotification(Browser browser, CefString host) {
    if (IsCorrectBrowser(browser)) {
        currentWarning = MainWindow::notification;
        ui->warningLabel->setText(QString::fromStdString(host.ToString()) + " wants to send you notifications.");

        ui->warningOk->setVisible(true);
        ui->warningCancel->setVisible(true);
        ui->warningOk->setText("Allow");
        ui->warningCancel->setText("Don't Allow");
        ui->warningFrame->setVisible(true);
    }
}

void MainWindow::MprisStateChanged(Browser browser, bool isOn) {
    if (indexOfBrowser(browser) != -1) {
        if (isOn) {
            if (tabBar->tabButton(indexOfBrowser(browser), QTabBar::LeftSide) == NULL) {
                QPushButton* button = new QPushButton;
                button->setIcon(QIcon::fromTheme("media-playback-pause"));
                button->setStyleSheet("padding: 0px; border: none;");
                button->setFlat(true);

                connect(button, &QPushButton::clicked, [=]() {
                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisPlayPause");
                    browser.get()->SendProcessMessage(PID_RENDERER, message);
                });

                tabBar->setTabButton(indexOfBrowser(browser), QTabBar::LeftSide, button);
            }
        } else {
            tabBar->setTabButton(indexOfBrowser(browser), QTabBar::LeftSide, NULL);
        }
    }
}

void MainWindow::MprisPlayingStateChanged(Browser browser, bool isPlaying) {
    if (indexOfBrowser(browser) != -1) {
        if (tabBar->tabButton(indexOfBrowser(browser), QTabBar::LeftSide) != NULL) {
            QIcon icon;
            if (isPlaying) {
                icon = QIcon::fromTheme("media-playback-pause");
            } else {
                icon = QIcon::fromTheme("media-playback-start");
            }
            ((QPushButton*) tabBar->tabButton(indexOfBrowser(browser), QTabBar::LeftSide))->setIcon(icon);
        }
    }
}

void MainWindow::NewDownload(Browser browser, CefRefPtr<CefDownloadItem> download_item) {
    if (indexOfBrowser(browser) != -1) {
        DownloadFrame* frame = new DownloadFrame(download_item, this);
        connect(signalBroker, SIGNAL(DownloadUpdated(Browser,CefRefPtr<CefDownloadItem>,CefRefPtr<CefDownloadItemCallback>)), frame, SLOT(DownloadUpdated(Browser,CefRefPtr<CefDownloadItem>,CefRefPtr<CefDownloadItemCallback>)));
        ui->downloadItemArea->layout()->addWidget(frame);

        if (ui->downloadItemAreaWidget->isVisible() == false) {
            ui->downloadItemAreaWidget->setFixedHeight(0);
            ui->downloadItemAreaWidget->setVisible(true);

            QVariantAnimation* animation = new QVariantAnimation;
            animation->setStartValue(0);
            animation->setEndValue(ui->downloadItemAreaWidget->sizeHint().height());
            animation->setDuration(500);
            animation->setEasingCurve(QEasingCurve::OutCubic);
            connect(animation, &QVariantAnimation::valueChanged, [=](QVariant value) {
                ui->downloadItemAreaWidget->setFixedHeight(value.toInt());
            });
            connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
            animation->start();
        }

        runningDownloads++;
        connect(frame, &DownloadFrame::Completed, [=]() {
            runningDownloads--;
        });
        connect(frame, &DownloadFrame::destroyed, [=]() {
            if (ui->downloadItemArea->layout()->count() == 1) {
                ui->downloadItemAreaWidget->setVisible(false);
            }
        });
    }
}

void MainWindow::BeforeDownload(Browser browser, CefRefPtr<CefDownloadItem> download_item, const CefString &suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback) {
    if (indexOfBrowser(browser) != -1) {
        /*QFileDialog* dialog = new QFileDialog;
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        dialog->selectFile(QString::fromStdString(suggested_name.ToString()));

        QStringList mimeTypeFilters;
        mimeTypeFilters.append(QString::fromStdString(download_item.get()->GetMimeType().ToString()));
        dialog->setMimeTypeFilters(mimeTypeFilters);
        dialog->setWindowModality(Qt::NonModal);

        if (dialog->exec() == QFileDialog::Accepted) {
            dialog->show();
            dialog->reject();
            emit signalBroker->NewDownload(browser, download_item);
            callback.get()->Continue(dialog->selectedFiles().first().toStdString(), false);
        }*/

        //Default to ~/Downloads/[filename]
        emit signalBroker->NewDownload(browser, download_item);
        QString selection = QDir::homePath() + "/Downloads/" + QString::fromStdString(suggested_name);
        callback.get()->Continue(selection.toStdString(), false);
    }
}

void MainWindow::on_findText_returnPressed()
{
    ui->findNext->click();
}

void MainWindow::on_actionFind_triggered()
{
    if (ui->findPanel->isVisible()) {
        ui->findClose->click();
    } else {
        ui->findPanel->setVisible(true);
        ui->findText->setFocus();
    }
}

void MainWindow::on_findNext_clicked()
{
    browser().get()->GetHost().get()->Find(0, ui->findText->text().toStdString(), true, false, false);
}

void MainWindow::on_findClose_clicked()
{
    browser().get()->GetHost().get()->StopFinding(true);
    ui->findPanel->setVisible(false);
}

void MainWindow::on_findBack_clicked()
{
    browser().get()->GetHost().get()->Find(0, ui->findText->text().toStdString(), false, false, false);
}

void MainWindow::on_findText_textChanged(const QString &arg1)
{
    ui->findNext->click();
}

void MainWindow::on_actionHistory_triggered()
{
    if (QString::fromStdString(browser().get()->GetMainFrame().get()->GetURL().ToString()) == settings.value("browser/home", "theweb://newtab").toString()) {
        browser().get()->GetMainFrame().get()->LoadURL("theweb://history");
    } else {
        CefWindowInfo windowInfo;
        CefBrowserSettings settings = defaultBrowserSettings;

        Browser browser = CefBrowserHost::CreateBrowserSync(windowInfo, handler, "theweb://history", settings, CefRefPtr<CefRequestContext>());

        createNewTab(browser);
    }
}

void MainWindow::setPopup(CefPopupFeatures features) {
    tabBar->setVisible(false);
    ui->actionGo_Back->setVisible(false);
    ui->actionGo_Forward->setVisible(false);
    ui->actionReload->setVisible(false);
    ui->actionNew_Tab->setVisible(false);
    menuButtonAction->setVisible(false);
    ui->spaceSearch->setReadOnly(true);

    QSize size(features.width, features.width);
    if (size.height() != 0 && size.width() != 0) {
        if (size.height() < 100) size.setHeight(100);
        if (size.width() < 100) size.setWidth(100);

        if (features.resizable) {
            this->resize(size);
        } else {
            this->setFixedSize(size);
        }
    }
}

void MainWindow::OpenURLFromTab(Browser browser, CefRefPtr<CefFrame> frame, const CefString &target_url, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture) {
    if (indexOfBrowser(browser) != -1) {
        switch (target_disposition) {
        case WOD_NEW_FOREGROUND_TAB:
        {
            Browser newBrowser;
            if (isOblivion) {
                CefBrowserSettings settings = defaultBrowserSettings;
                settings.application_cache = STATE_DISABLED;

                CefRequestContextSettings contextSettings;
                CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(contextSettings, new OblivionRequestContextHandler);
                context.get()->RegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
                newBrowser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, target_url, settings, context);
            } else {
                newBrowser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, target_url, defaultBrowserSettings, CefRefPtr<CefRequestContext>());
            }
            createNewTab(newBrowser);
        }
            break;
        case WOD_NEW_BACKGROUND_TAB:
        {
            Browser newBrowser;
            if (isOblivion) {
                CefBrowserSettings settings = defaultBrowserSettings;
                settings.application_cache = STATE_DISABLED;

                CefRequestContextSettings contextSettings;
                CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(contextSettings, new OblivionRequestContextHandler);
                context.get()->RegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
                newBrowser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, target_url, settings, context);
            } else {
                newBrowser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, target_url, defaultBrowserSettings, CefRefPtr<CefRequestContext>());
            }
            createNewTab(newBrowser, true);
        }
            break;
        case WOD_NEW_WINDOW:
        {
            Browser newBrowser;
            if (isOblivion) {
                CefBrowserSettings settings = defaultBrowserSettings;
                settings.application_cache = STATE_DISABLED;

                CefRequestContextSettings contextSettings;
                CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(contextSettings, new OblivionRequestContextHandler);
                context.get()->RegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
                newBrowser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, target_url, settings, context);
            } else {
                newBrowser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, target_url, defaultBrowserSettings, CefRefPtr<CefRequestContext>());
            }

            MainWindow* window = new MainWindow(newBrowser, isOblivion);
            window->show();
        }
            break;
        }
    }
}

void MainWindow::FileDialog(Browser browser, CefDialogHandler::FileDialogMode mode, const CefString &title, const CefString &default_file_path, const std::vector<CefString> &accept_filters, int selected_accept_filter, CefRefPtr<CefFileDialogCallback> callback) {
    if (indexOfBrowser(browser) != -1) {
        if ((mode & FILE_DIALOG_TYPE_MASK) == FILE_DIALOG_OPEN) {
            ui->SelectFilePicker->startSelectFile(callback, FilePicker::single);
        } else if ((mode & FILE_DIALOG_TYPE_MASK) == FILE_DIALOG_OPEN_MULTIPLE) {
            ui->SelectFilePicker->startSelectFile(callback, FilePicker::multiple);
        } else if ((mode & FILE_DIALOG_TYPE_MASK) == FILE_DIALOG_OPEN_FOLDER) {
            ui->SelectFilePicker->startSelectFile(callback, FilePicker::singleFolder);
        }
        insertIntoMetadata(browser, "filePicker", true);
        updateCurrentBrowserDisplay();
    }
}

void MainWindow::on_SelectFilePicker_fileDone()
{
    removeFromMetadata(browser(), "filePicker");
    updateCurrentBrowserDisplay();
}

void MainWindow::on_actionPrint_triggered()
{
    browser().get()->GetHost().get()->Print();
}

void MainWindow::PrintDialog(Browser browser, QPrinter* printer, bool has_selection, CefRefPtr<CefPrintDialogCallback> callback) {
    if (indexOfBrowser(browser) != -1) {
        ui->printDestination->clear();
        for (QString printerName : QPrinterInfo::availablePrinterNames()) {
            ui->printDestination->addItem(printerName);
        }
        ui->printDestination->setCurrentText(printer->printerName());

        ui->printCopies->setValue(printer->copyCount());
        ui->printDoubleSided->setChecked(printer->doubleSidedPrinting());
        if (printer->orientation() == QPrinter::Portrait) {
            ui->printOrientationPortrait->setChecked(true);
        } else {
            ui->printOrientationLandscape->setChecked(true);
        }
        ui->printGreyscale->setChecked(printer->colorMode() == QPrinter::GrayScale);

        QVariantList printMetadata;
        printMetadata.append(QVariant::fromValue(callback));
        printMetadata.append(QVariant::fromValue(printer));

        insertIntoMetadata(browser, "print", printMetadata);
        updateCurrentBrowserDisplay();
    }
}

void MainWindow::on_printCancel_clicked()
{
    QVariantList printMetadata = browserMetadata.at(indexOfBrowser(browser())).value("print").toList();
    printMetadata.first().value<CefRefPtr<CefPrintDialogCallback>>().get()->Cancel();
    removeFromMetadata(browser(), "print");
    updateCurrentBrowserDisplay();
}

void MainWindow::on_printButton_clicked()
{
    QVariantList printMetadata = browserMetadata.at(indexOfBrowser(browser())).value("print").toList();
    QPrinter* printer = printMetadata.at(1).value<QPrinter*>();

    printer->setPrinterName(ui->printDestination->currentText());
    printer->setCopyCount(ui->printCopies->value());
    printer->setDoubleSidedPrinting(ui->printDoubleSided->isChecked());
    printer->setOrientation(ui->printOrientationPortrait->isChecked() ? QPrinter::Portrait : QPrinter::Landscape);
    printer->setColorMode(ui->printGreyscale->isChecked() ? QPrinter::GrayScale : QPrinter::Color);

    printMetadata.first().value<CefRefPtr<CefPrintDialogCallback>>().get()->Continue(CefEngine::getCefPrinterSettings(printer));

    removeFromMetadata(browser(), "print");
    updateCurrentBrowserDisplay();
}

bool MainWindow::oblivionWindow() {
    return this->isOblivion;
}

void MainWindow::navigate(int index, QString url) {
}

void MainWindow::StatusMessage(Browser browser, const CefString &value) {
    if (indexOfBrowser(browser) != -1) {
        ui->spaceSearch->setHoverText(QString::fromStdString(value));
    }
}
