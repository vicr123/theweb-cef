#include "mainwindow.h"
#include "ui_mainwindow.h"

extern CefHandler* handler;
extern SignalBroker* signalBroker;
extern void QuitApp(int exitCode = 0);
extern QTimer cefEventLoopTimer;
extern QStringList certErrorUrls;

MainWindow::MainWindow(Browser newBrowser, bool isOblivion, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->isOblivion = isOblivion;

    tabBar = new QTabBar();
    tabBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    tabBar->setTabsClosable(true);
    tabBar->setShape(QTabBar::TriangularNorth);
    tabBar->setExpanding(false);
    ((QBoxLayout*) ui->centralwidget->layout())->insertWidget(ui->centralwidget->layout()->indexOf(ui->errorFrame), tabBar);

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

    this->resize(700, 700);
    ui->errorFrame->setVisible(false);
    ui->warningFrame->setVisible(false);
    ui->menubar->setVisible(false);
    ui->JsDialogFrame->setVisible(false);
    ui->AuthFrame->setVisible(false);
    ui->fraudFrame->setVisible(false);
    ui->fraudExtraFrame->setVisible(false);
    ui->securityEVName->setVisible(false);
    ui->securityInfoFrame->setVisible(false);
    ui->badCertificateFrame->setVisible(false);
    ui->certMoreInfo->setVisible(false);

    QPalette oldFraudContentPalette = ui->fraudContent->palette();
    oldFraudContentPalette.setColor(QPalette::Window, oldFraudContentPalette.color(QPalette::Window));
    QPalette oldFraudPalette = ui->fraudFrame->palette();
    oldFraudPalette.setColor(QPalette::Window, QColor::fromRgb(100, 0, 0));
    ui->fraudFrame->setPalette(oldFraudPalette);
    ui->fraudContent->setPalette(oldFraudContentPalette);

    ui->securityPadlock->setPixmap(QIcon::fromTheme("text-html").pixmap(16, 16));

    ui->toolBar->addWidget(ui->securityFrame);
    ui->toolBar->addWidget(ui->spaceSearch);

    ui->securityFrame->setAutoFillBackground(true);

    QMenu* menu = new QMenu();
    menu->addAction(ui->actionNew_Tab);
    menu->addAction(ui->actionNew_Window);
    menu->addAction(ui->actionNew_Oblivion_Window);
    menu->addSeparator();
    menu->addAction(ui->actionSettings);
    menu->addAction(ui->actionAbout_theWeb);
    menu->addSeparator();
    menu->addAction(ui->actionClose_Tab);
    menu->addAction(ui->actionExit);

    QToolButton* menuButton = new QToolButton();
    menuButton->setPopupMode(QToolButton::InstantPopup);
    if (isOblivion) {
        menuButton->setIcon(QIcon(":/icons/oblivionIcon"));
        QPalette searchPalette = ui->spaceSearch->palette();
        QColor temp;
        temp = searchPalette.color(QPalette::Base);
        searchPalette.setColor(QPalette::Base, searchPalette.color(QPalette::Text));
        searchPalette.setColor(QPalette::Text, temp);
        ui->spaceSearch->setPalette(searchPalette);
    } else {
        menuButton->setIcon(QIcon(":/icons/icon"));
    }
    menuButton->setMenu(menu);
    ui->toolBar->insertWidget(ui->actionGo_Back, menuButton);

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
    connect(signalBroker, SIGNAL(ReloadSettings()), this, SLOT(ReloadSettings()));

    createNewTab(newBrowser);

    connect(tabBar, &QTabBar::currentChanged, [=](int currentIndex) {
        if (currentIndex != -1) {
            Browser newBrowser = browserList.at(currentIndex);
            ui->browserStack->setCurrentIndex(currentIndex);
            ui->spaceSearch->setCurrentUrl(QUrl(QString::fromStdString(newBrowser.get()->GetMainFrame().get()->GetURL())));
        }

        updateCurrentBrowserDisplay();
    });
    connect(tabBar, &QTabBar::tabCloseRequested, [=](int closeIndex) {
        browserList.at(closeIndex).get()->GetHost().get()->CloseBrowser(false);
    });
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
    browserIcons.append(QIcon());
    tabBar->addTab("New Tab");

    CefWindowInfo windowInfo;
    CefBrowserSettings settings;

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
    browserWidget = QWidget::createWindowContainer(window);
    browserWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    ui->browserStack->addWidget(browserWidget);
    browserList.append(browser);

    if (!openInBackground) {
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

    if (event->isAccepted()) {
        browserWidget->close();
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

        tabBar->setTabText(indexOfBrowser(browser), tabBar->fontMetrics().elidedText("Well, this is strange...", Qt::ElideRight, 200));
        tabBar->setTabData(indexOfBrowser(browser), "Well, this is strange...");

        QIcon icon = QIcon::fromTheme("dialog-error");
        tabBar->setTabIcon(indexOfBrowser(browser), QIcon(icon));
        browserIcons.replace(indexOfBrowser(browser), icon);

        updateCurrentBrowserDisplay();
    }
}

void MainWindow::TitleChanged(Browser browser, const CefString& title) {
    if (indexOfBrowser(browser) != -1) {
        tabBar->setTabText(indexOfBrowser(browser), tabBar->fontMetrics().elidedText(QString::fromStdString(title.ToString()), Qt::ElideRight, 200));
        tabBar->setTabData(indexOfBrowser(browser), QString::fromStdString(title.ToString()));
        if (IsCorrectBrowser(browser)) {
            this->setWindowTitle(QString::fromStdString(title.ToString()).append(" - theWeb"));
        }
    }
}

void MainWindow::on_spaceSearch_returnPressed()
{
    QString urlToLoad;
    if (ui->spaceSearch->text().contains(".") || ui->spaceSearch->text().contains("/") || ui->spaceSearch->text().contains("\\")) {
        QUrl urlParser = QUrl::fromUserInput(ui->spaceSearch->text());
        if (!urlParser.isEmpty() || urlParser.scheme() == "theweb" || urlParser.scheme() == "chrome") {
            urlToLoad = ui->spaceSearch->text();
        } else {
            urlToLoad = "http://www.google.com/search#q=" + ui->spaceSearch->text().replace(" ", "+");
        }
    } else {
        urlToLoad = "http://www.google.com/search#q=" + ui->spaceSearch->text().replace(" ", "+");
    }
    browser().get()->GetMainFrame().get()->LoadURL(urlToLoad.toStdString());
}

void MainWindow::AddressChange(Browser browser, CefRefPtr<CefFrame> frame, const CefString &url) {
    if (indexOfBrowser(browser) != -1 && frame.get()->IsMain()) {
        QStringList securityMetadata;

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
            securityMetadata.append("You are viewing content generated by theWeb.");
        } else {
            if (currentUrl.scheme() == "https") {
                if (certErrorUrls.contains(QString::fromStdString(url.ToString()))) {
                    securityMetadata.append("certerr");
                    securityMetadata.append("Insecure Connection");
                    securityMetadata.append("This connection may have been intercepted.");
                } else {
                    securityMetadata.append("secure");
                    securityMetadata.append("");
                    securityMetadata.append("Please wait...");

                    QSslSocket *sslSock = new QSslSocket();
                    connect(sslSock, &QSslSocket::encrypted, [=]() {
                        QStringList securityMetadata;

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

                            qDebug() << "Number of Certificate Extensions: " << certificate.extensions().count();
                            for (QSslCertificateExtension ext : certificate.extensions()) {
                                if (!isEv) {
                                    if (ext.oid() == "2.5.29.32") {
                                        qDebug() << ext.value().toByteArray();
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

                        if (isEv) {
                            QString commonName = "";
                            QString countryName = "";
                            if (certificate.subjectInfo(QSslCertificate::CommonName).count() != 0) {
                                commonName = certificate.subjectInfo(QSslCertificate::Organization).first();
                            }

                            if (certificate.subjectInfo(QSslCertificate::CountryName).count() != 0) {
                                countryName = certificate.subjectInfo(QSslCertificate::CountryName).first();
                            }

                            securityMetadata.append("ev");
                            securityMetadata.append(commonName + " [" + countryName + "]");
                            securityMetadata.append("This connection is secure, and the company has been verified by <b>" +
                                                    certificate.issuerInfo(QSslCertificate::CommonName).first() + "</b> to be <b>" +
                                                    commonName + "</b>");
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
                            securityMetadata.append("This connection is secure, and has been verified by <b>" + certificate.issuerInfo(QSslCertificate::CommonName).first() + "</b>");
                        }

                        insertIntoMetadata(browser, "security", securityMetadata);
                        updateCurrentBrowserDisplay();
                    });
                    connect(sslSock, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
                        [=](QAbstractSocket::SocketError socketError){
                        qDebug() << socketError;
                    });
                    sslSock->connectToHostEncrypted(currentUrl.host(), currentUrl.port(443));
                }
            } else {
                securityMetadata.append("unsecure");
                securityMetadata.append("");
                securityMetadata.append("This connection has not been encrypted.");

            }

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
                        threatMetadata.append("This website may trick you into doing something into revealing your "
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

        insertIntoMetadata(browser, "security", securityMetadata);
        updateCurrentBrowserDisplay();

        //Flush cookies (just to make sure)
        if (CefCookieManager::GetGlobalManager(NULL).get() != NULL) {
            CefCookieManager::GetGlobalManager(NULL).get()->FlushStore(NULL);
        }
    }
}

void MainWindow::insertIntoMetadata(Browser browser, QString key, QVariant value) {
    QVariantMap currentBrowserMap = browserMetadata.at(indexOfBrowser(browser));
    currentBrowserMap.insert(key, value);
    browserMetadata.replace(indexOfBrowser(browser), currentBrowserMap);
}

void MainWindow::removeFromMetadata(Browser browser, QString key) {
    QVariantMap currentBrowserMap = browserMetadata.at(indexOfBrowser(browser));
    if (currentBrowserMap.keys().contains(key)) {
        currentBrowserMap.remove(key);
        browserMetadata.replace(indexOfBrowser(browser), currentBrowserMap);
    }
}

void MainWindow::FullscreenModeChange(Browser browser, bool fullscreen) {
    if (IsCorrectBrowser(browser)) {
        static bool wasMaximized = false;
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
        ; //do nothing
    }
    ui->warningFrame->setVisible(false);

    currentWarning = none;
}

void MainWindow::on_warningCancel_clicked()
{
    switch (currentWarning) {
    case fullscreen:
        browser().get()->GetMainFrame().get()->ExecuteJavaScript("document.webkitExitFullscreen()", "", 0);
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
        if (errorCode != ERR_ABORTED && frame.get()->IsMain()) {
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
            case ERR_UNKNOWN_URL_SCHEME:
                errorDisplayMetadata.append("Unknown URL Scheme");
                break;
            case ERR_INVALID_URL:
                errorDisplayMetadata.append("Invalid URL");
                break;
            case -21: //Network change detected
                errorDisplayMetadata.append("The internet connection changed while we were trying to connect to the page.");
                break;
            default:
                errorDisplayMetadata.append(QString::fromStdString(failedUrl.ToString()) + " may be down or it may have moved somewhere else. " );
            }

            insertIntoMetadata(browser, "error", errorDisplayMetadata);

            tabBar->setTabText(indexOfBrowser(browser), tabBar->fontMetrics().elidedText(errorDisplayMetadata.at(0), Qt::ElideRight, 200));
            tabBar->setTabData(indexOfBrowser(browser), errorDisplayMetadata.at(0));

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

        //Actually close the window
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
    browserMetadata.at(indexOfBrowser(browser())).value("js").toList().at(2).value<CefRefPtr<CefJSDialogCallback>>().get()->Continue(true, ui->JsDialogPrompt->text().toStdString());
    removeFromMetadata(browser(), "js");
    updateCurrentBrowserDisplay();
}

void MainWindow::on_JsDialogCancel_clicked()
{
    ui->JsDialogFrame->setVisible(false);
    ui->browserStack->setVisible(true);
    browserMetadata.at(indexOfBrowser(browser())).value("js").toList().at(2).value<CefRefPtr<CefJSDialogCallback>>().get()->Continue(false, "");
    removeFromMetadata(browser(), "js");
    updateCurrentBrowserDisplay();
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
        AuthMetadata.append("Log in to " + QString::fromStdString(host.ToString()));
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
        if (!user_gesture) {
            if (QMessageBox::information(this, "Open pop-up?", "This webpage is requesting that " + QString::fromStdString(target_url.ToString()) + " be opened in a new window. Is that OK?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
                return;
            }
        }

        Browser newBrowser;
        if (isOblivion) {
            CefBrowserSettings settings;
            settings.application_cache = STATE_DISABLED;

            CefRequestContextSettings contextSettings;
            CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(contextSettings, new OblivionRequestContextHandler);
            context.get()->RegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
            newBrowser = browser.get()->GetHost().get()->CreateBrowserSync(*windowInfo, CefRefPtr<CefHandler>(handler), target_url, settings, context);
        } else {
            newBrowser = browser.get()->GetHost().get()->CreateBrowserSync(*windowInfo, CefRefPtr<CefHandler>(handler), target_url, CefBrowserSettings(), CefRefPtr<CefRequestContext>());
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

    }
}

void MainWindow::on_actionLimit_to_60_fps_triggered()
{
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(true);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(1000 / 60);
}

void MainWindow::on_actionLimit_to_30_fps_triggered()
{
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(true);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(1000 / 30);
}

void MainWindow::on_actionLimit_to_15_fps_triggered()
{
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(true);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(1000 / 15);
}

void MainWindow::on_actionLimit_to_1_fps_triggered()
{
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(true);
    cefEventLoopTimer.setInterval(1000);
}

void MainWindow::on_actionDon_t_Limit_triggered()
{
    ui->actionDon_t_Limit->setChecked(true);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(0);
}

void MainWindow::on_actionAbout_theWeb_triggered()
{
    if (QString::fromStdString(browser().get()->GetMainFrame().get()->GetURL().ToString()) == settings.value("browser/home", "theweb://newtab").toString()) {
        browser().get()->GetMainFrame().get()->LoadURL("theweb://theweb");
    } else {
        CefWindowInfo windowInfo;
        CefBrowserSettings settings;

        Browser browser = CefBrowserHost::CreateBrowserSync(windowInfo, CefRefPtr<CefHandler>(handler), "theweb://theweb", settings, CefRefPtr<CefRequestContext>());

        createNewTab(browser);
    }
}

void MainWindow::on_actionSettings_triggered()
{
    if (QString::fromStdString(browser().get()->GetMainFrame().get()->GetURL().ToString()) == settings.value("browser/home", "theweb://newtab").toString()) {
        browser().get()->GetMainFrame().get()->LoadURL("theweb://settings");
    } else {
        CefWindowInfo windowInfo;
        CefBrowserSettings settings;

        Browser browser = CefBrowserHost::CreateBrowserSync(windowInfo, CefRefPtr<CefHandler>(handler), "theweb://settings", settings, CefRefPtr<CefRequestContext>());

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
        this->close();
    }
    ui->fraudFrame->setVisible(false);
    ui->browserStack->setVisible(true);
    removeFromMetadata(browser(), "threat");
}

void MainWindow::on_securityFrame_clicked()
{
    if (ui->securityInfoFrame->isVisible()) {
        ui->securityInfoFrame->setVisible(false);
    } else {
        ui->securityInfoFrame->setVisible(true);
    }
}

void MainWindow::CertificateError(Browser browser, cef_errorcode_t cert_error, const CefString &request_url, CefRefPtr<CefSSLInfo> ssl_info, CefRefPtr<CefRequestCallback> callback) {
    if (IsCorrectBrowser(browser)) {
        QVariantList certErrorMetadata;


        switch (cert_error) {
        case ERR_CERT_COMMON_NAME_INVALID:
            certErrorMetadata.append("You're trying to connect to <b>" + QUrl(QString::fromStdString(request_url.ToString())).host() + "</b> but the server presented itself as <b>" +
                                      QString::fromStdString(ssl_info.get()->GetSubject().get()->GetDisplayName().ToString()) + "</b>.");
            break;
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

        //certCallback = callback;
        certErrorMetadata.append(QVariant::fromValue(callback));

        certErrorUrls.append(QString::fromStdString(request_url.ToString()));

        QStringList securityMetadata;
        securityMetadata.append("certerr");
        securityMetadata.append("Insecure Connection");
        securityMetadata.append("This connection may have been intercepted.");
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
        ui->certIgnore->setText("Continue Anyway");
    }
}

void MainWindow::on_certificateBack_clicked()
{
    ui->actionGo_Back->trigger();
    ui->badCertificateFrame->setVisible(false);
    ui->browserStack->setVisible(true);
}

void MainWindow::ReloadSettings() {
    if (settings.value("browser/toolbarOnBottom").toBool()) {
        this->addToolBar(Qt::BottomToolBarArea, ui->toolBar);
        tabBar->setShape(QTabBar::RoundedSouth);
        ((QBoxLayout*) ui->centralwidget->layout())->addWidget(tabBar);
        ((QBoxLayout*) ui->centralwidget->layout())->addWidget(ui->securityInfoFrame);
    } else {
        this->addToolBar(Qt::TopToolBarArea, ui->toolBar);
        tabBar->setShape(QTabBar::RoundedNorth);
        ((QBoxLayout*) ui->centralwidget->layout())->insertWidget(ui->centralwidget->layout()->indexOf(ui->warningFrame), ui->securityInfoFrame);
        ((QBoxLayout*) ui->centralwidget->layout())->insertWidget(ui->centralwidget->layout()->indexOf(ui->warningFrame), tabBar);
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

            ui->securityText->setText(securityMetadata.at(2));
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

        if (metadata.keys().contains("js")) {
            QVariantList JsMetadata = metadata.value("js").toList();

            ui->JsDialogText->setText(JsMetadata.at(1).toString());
            if (JsMetadata.at(0).toString() == "unload") {
                ui->JsDialogFrame->setVisible(true);
                if (JsMetadata.at(3).toBool()) {
                    ui->JsDialogOk->setText("Reload anyway");
                    ui->JsDialogCancel->setText("Don't Reload");
                } else {
                    ui->JsDialogOk->setText("Leave anyway");
                    ui->JsDialogCancel->setText("Don't leave");
                }
                ui->JsBeforeLeaveTitle->setVisible(true);
                ui->JsDialogPrompt->setVisible(false);
            } else {
                ui->JsDialogFrame->setVisible(true);
                ui->JsDialogOk->setText("OK");
                ui->JsDialogCancel->setText("Cancel");
                ui->JsBeforeLeaveTitle->setVisible(false);

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

            ui->badCertificateFrame->setVisible(true);
            showBrowserStack = false;
        } else {
            ui->badCertificateFrame->setVisible(false);
            ui->certMoreInfo->setVisible(false);
            ui->certIgnore->setText("More Info");
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
        } else {
            ui->fraudFrame->setVisible(false);
        }

        if (showBrowserStack) {
            ui->browserStack->setVisible(true);
        } else {
            ui->browserStack->setVisible(false);
        }
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
        if (os_event->xkey.type == KeyPress) {
            if (os_event->xkey.state == ControlMask) {
                if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_T)) { //New Tab
                    ui->actionNew_Tab->trigger();
                } else if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_N)) { //New Window
                    ui->actionNew_Window->trigger();
                } else if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_W)) { //Close Tab
                    ui->actionClose_Tab->trigger();
                } else if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_comma)) { //Settings
                    ui->actionSettings->trigger();
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
