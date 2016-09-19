#include "mainwindow.h"
#include "ui_mainwindow.h"

extern CefHandler* handler;
extern SignalBroker* signalBroker;
extern void QuitApp(int exitCode = 0);
extern QTimer cefEventLoopTimer;
extern QStringList certErrorUrls;

MainWindow::MainWindow(Browser newBrowser, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

    ReloadSettings();

    ui->securityFrame->setAutoFillBackground(true);

    QMenu* menu = new QMenu();
    menu->addAction(ui->actionNew_Window);
    menu->addSeparator();
    menu->addAction(ui->actionSettings);
    menu->addAction(ui->actionAbout_theWeb);
    menu->addSeparator();
    menu->addAction(ui->actionExit);

    QToolButton* menuButton = new QToolButton();
    menuButton->setPopupMode(QToolButton::InstantPopup);
    menuButton->setIcon(QIcon(":/icons/icon"));
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
    connect(signalBroker, SIGNAL(ReloadSettings()), this, SLOT(ReloadSettings()));

    CefWindowInfo windowInfo;
    //windowInfo.SetAsChild(0, CefRect(0, 0, 100, 100));

    CefBrowserSettings settings;

    if (newBrowser.get() == 0) {
        browser = CefBrowserHost::CreateBrowserSync(windowInfo, CefRefPtr<CefHandler>(handler), "http://www.google.com/", settings, CefRefPtr<CefRequestContext>());
    } else {
        browser = newBrowser;
    }

    QWindow* window = QWindow::fromWinId(browser.get()->GetHost()->GetWindowHandle());
    browserWidget = QWidget::createWindowContainer(window);
    browserWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    ((QBoxLayout*) ui->centralwidget->layout())->insertWidget(ui->centralwidget->layout()->indexOf(ui->loadingProgressBar), browserWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (!handler->canClose(browser)) {
        browser.get()->GetHost().get()->CloseBrowser(false);
        event->ignore();
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
    browser.get()->GoBack();
}

void MainWindow::on_actionGo_Forward_triggered()
{
    browser.get()->GoForward();
}

void MainWindow::on_reloadErrorButton_clicked()
{
    ui->actionReload->trigger();
    ui->errorFrame->setVisible(false);
    browserWidget->setVisible(true);
}

void MainWindow::on_actionReload_triggered()
{
    browser.get()->ReloadIgnoreCache();
}

bool MainWindow::IsCorrectBrowser(Browser browser) {
    if (browser.get() != NULL && this->browser.get() != NULL) {
        return browser.get()->GetIdentifier() == this->browser.get()->GetIdentifier();
    } else {
        return false;
    }
}

void MainWindow::RenderProcessTerminated(Browser browser, CefRequestHandler::TerminationStatus status) {
    if (IsCorrectBrowser(browser)) {
        ui->errorTitle->setText("Well, this is strange.");
        switch (status) {
        case TS_PROCESS_CRASHED:
            ui->errorText->setText("The window crashed.");
            break;
        case TS_PROCESS_WAS_KILLED:
            ui->errorText->setText("The window was closed by the system.");
            break;
        case TS_ABNORMAL_TERMINATION:
            ui->errorText->setText("The window closed abnormally.");
        }

        ui->errorFrame->setVisible(true);
        browserWidget->setVisible(false);
    }
}

void MainWindow::TitleChanged(Browser browser, const CefString& title) {
    if (IsCorrectBrowser(browser)) {
        this->setWindowTitle(QString::fromStdString(title.ToString()).append(" - theWeb"));
    }
}

void MainWindow::on_spaceSearch_returnPressed()
{
    browser.get()->GetMainFrame().get()->LoadURL(ui->spaceSearch->text().toStdString());
}

void MainWindow::AddressChange(Browser browser, CefRefPtr<CefFrame> frame, const CefString &url) {
    if (IsCorrectBrowser(browser) && frame.get()->IsMain()) {
        QUrl currentUrl(QString::fromStdString(url.ToString()));
        ui->fraudFrame->setVisible(false);
        ui->fraudExtraFrame->setVisible(false);
        ui->fraudIgnore->setText("More Info");
        ui->badCertificateFrame->setVisible(false);
        ui->certMoreInfo->setVisible(false);
        ui->certIgnore->setText("More Info");

        ui->spaceSearch->setCurrentUrl(currentUrl);

        if (QUrl(QString::fromStdString(url.ToString())).scheme() == "theweb") {
            ui->securityEVName->setText("theWeb Generated Content");
            ui->securityFrame->setStyleSheet("");
            ui->securityEVName->setVisible(true);
            ui->securityPadlock->setPixmap(QIcon(":/icons/icon").pixmap(16, 16));

            ui->securityText->setText("You are viewing content generated by theWeb.");
        } else {
            if (currentUrl.scheme() == "https") {
                if (certErrorUrls.contains(QString::fromStdString(url.ToString()))) {
                    ui->securityEVName->setText("Insecure Connection!");
                    ui->securityEVName->setVisible(true);
                    ui->securityFrame->setStyleSheet("background-color: #640000; color: white;");
                    ui->securityPadlock->setPixmap(QIcon(":/icons/badsecure").pixmap(16, 16));

                    ui->securityText->setText("This connection may have been intercepted.");
                } else {
                    QSslSocket *sslSock = new QSslSocket();
                    connect(sslSock, &QSslSocket::encrypted, [=]() {
                        QSslCertificate certificate = sslSock->peerCertificate();
                        QList<QSslCertificate> certificateChain = sslSock->peerCertificateChain();
                        sslSock->close();
                        sslSock->deleteLater();

                        bool isEv = false;
                        //if (certificateChain.count() > 2) {
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

                            ui->securityEVName->setText(commonName + " [" + countryName + "]");
                            ui->securityEVName->setVisible(true);
                            ui->securityFrame->setStyleSheet("background-color: #006400; color: white;");
                            ui->securityPadlock->setPixmap(QIcon(":/icons/lock-d").pixmap(16, 16));

                            ui->securityText->setText("This connection is secure, and the company has been verified by <b>" +
                                                      certificate.issuerInfo(QSslCertificate::CommonName).first() + "</b> to be <b>" +
                                                      commonName + "</b>");
                        } else {
                            ui->securityEVName->setVisible(false);
                            ui->securityFrame->setStyleSheet("");

                            QColor panelColor = ui->securityFrame->palette().color(QPalette::Window);
                            if (((qreal) panelColor.red() + (qreal) panelColor.green() + (qreal) panelColor.red()) / (qreal) 3 < 127) {
                                ui->securityPadlock->setPixmap(QIcon(":/icons/lock-d").pixmap(16, 16));
                            } else {
                                ui->securityPadlock->setPixmap(QIcon(":/icons/lock-l").pixmap(16, 16));
                            }

                            ui->securityText->setText("This connection is secure, and has been verified by <b>" + certificate.issuerInfo(QSslCertificate::CommonName).first() + "</b>");
                        }
                    });
                    connect(sslSock, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
                        [=](QAbstractSocket::SocketError socketError){
                        qDebug() << socketError;
                    });
                    sslSock->connectToHostEncrypted(currentUrl.host(), currentUrl.port(443));
                }
            } else {
                ui->securityPadlock->setPixmap(QIcon::fromTheme("text-html").pixmap(16, 16));
                ui->securityEVName->setVisible(false);
                ui->securityFrame->setStyleSheet("");
                ui->securityText->setText("This connection has not been encrypted.");
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
                    QString threatType = object.value("matches").toArray().first().toObject().value("threatType").toString();
                    if (threatType == "MALWARE") {
                        ui->fraudExplanation->setText("This website may contain malware. <b>We suggest that you don't visit "
                                                      "this website.</b>");
                        ui->fraudExtraText->setText("Google Safe Browsing found malware on this site. Malware can cause your "
                                                    "PC to slow down or act erratically.");
                    } else if (threatType == "SOCIAL_ENGINEERING") {
                        ui->fraudExplanation->setText("This website may trick you into doing something into revealing your "
                                                      "personal information (such as passwords or credit card information) or "
                                                      "installing software that you may not want. <b>We suggest that you don't visit "
                                                      "this website and enter any personal information.</b>");
                        ui->fraudExtraText->setText("Google Safe Browsing found this site to be deceptive. These websites trick "
                                                    "users into doing something dangerous.");
                    }
                    ui->fraudFrame->setVisible(true);
                    browserWidget->setVisible(false);
                }
            });
            manager->post(request, requestBody.toUtf8());

        }

        //Flush cookies (just to make sure)
        CefCookieManager::GetGlobalManager(NULL).get()->FlushStore(NULL);
    }
}

void MainWindow::FullscreenModeChange(Browser browser, bool fullscreen) {
    if (IsCorrectBrowser(browser)) {
        static bool wasMaximized = false;
        if (fullscreen) {
            wasMaximized = this->isMaximized();
            ui->toolBar->setVisible(false);
            this->showFullScreen();

            currentWarning = MainWindow::fullscreen;
            QUrl currentUrl(QString::fromStdString(browser.get()->GetMainFrame().get()->GetURL().ToString()));
            ui->warningLabel->setText(currentUrl.host() + " is now full screen.");

            ui->warningOk->setVisible(true);
            ui->warningCancel->setVisible(true);
            ui->warningOk->setText("OK");
            ui->warningCancel->setText("Exit Full Screen");
            ui->warningFrame->setVisible(true);
        } else {
            ui->toolBar->setVisible(true);
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
        browser.get()->GetMainFrame().get()->ExecuteJavaScript("document.webkitExitFullscreen()", "", 0);
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
    if (IsCorrectBrowser(browser)) {
        browserWidget->setVisible(false);
        ui->JsDialogFrame->setVisible(true);
        ui->JsDialogOk->setText("OK");
        ui->JsDialogCancel->setText("Cancel");
        ui->JsDialogText->setText(QString::fromStdString(message_text.ToString()));

        switch (dialog_type) {
        case JSDIALOGTYPE_ALERT:
            ui->JsDialogCancel->setVisible(false);
            ui->JsDialogPrompt->setVisible(false);
            break;
        case JSDIALOGTYPE_CONFIRM:
            ui->JsDialogCancel->setVisible(true);
            ui->JsDialogPrompt->setVisible(false);
            break;
        case JSDIALOGTYPE_PROMPT:
            ui->JsDialogCancel->setVisible(true);
            ui->JsDialogPrompt->setText(QString::fromStdString(default_prompt_text));
            ui->JsDialogPrompt->setVisible(true);

            ui->JsDialogPrompt->setFocus();
            break;
        }
        this->JsDialogCallback = callback;
    }
}

void MainWindow::LoadingStateChange(Browser browser, bool isLoading, bool canGoBack, bool canGoForward) {
    if (IsCorrectBrowser(browser)) {
        ui->actionGo_Back->setEnabled(canGoBack);
        ui->actionGo_Forward->setEnabled(canGoForward);

        if (isLoading) {
            ui->errorFrame->setVisible(false);
            browserWidget->setVisible(true);
            ui->loadingProgressBar->setVisible(true);
        } else {
            ui->loadingProgressBar->setVisible(false);
        }
    }
}

void MainWindow::LoadError(Browser browser, CefRefPtr<CefFrame> frame, CefHandler::ErrorCode errorCode, const CefString &errorText, const CefString &failedUrl) {
    if (IsCorrectBrowser(browser)) {
        if (errorCode != ERR_ABORTED && frame.get()->IsMain()) {
            switch (qrand() % 7) {
            case 0:
                ui->errorTitle->setText("This isn't supposed to happen...");
                break;
            case 1:
                ui->errorTitle->setText("Can't connect to webpage");
                break;
            case 2:
                ui->errorTitle->setText("Well...");
                break;
            case 3:
                ui->errorTitle->setText("Well, that didn't work.");
                break;
            case 4:
                ui->errorTitle->setText("Something's not right.");
                break;
            case 5:
                ui->errorTitle->setText("Whoa!");
                break;
            case 6:
                ui->errorTitle->setText("Oops!");
                break;
            }


            switch (errorCode) {
            case ERR_NAME_NOT_RESOLVED:
                ui->errorText->setText("Couldn't find server");
                break;
            case ERR_TIMED_OUT:
                ui->errorText->setText("Server took too long to respond");
                break;
            case ERR_ACCESS_DENIED:
                ui->errorText->setText("Access Denied");
                break;
            case ERR_CACHE_MISS:
                ui->errorTitle->setText("Confirm Form Resubmission");
                ui->errorText->setText("To display this webpage, data that you entered previously needs to be sent again. To do so, "
                                       "click \"Reload.\" However, note that by doing this, you will repeat any action that this page "
                                       "performed.");
                break;
            case ERR_CONNECTION_RESET:
                ui->errorText->setText("The connection was reset");
                break;
            case ERR_CONNECTION_REFUSED:
                ui->errorText->setText("The server refused the connection");
                break;
            case ERR_CONNECTION_CLOSED:
            case ERR_EMPTY_RESPONSE:
                ui->errorText->setText("The server didn't send anything");
                break;
            case ERR_INTERNET_DISCONNECTED:
                ui->errorText->setText("Disconnected from the internet");
                break;
            case ERR_TOO_MANY_REDIRECTS:
                ui->errorText->setText("A redirect loop was detected");
                break;
            case ERR_UNSAFE_REDIRECT:
                ui->errorText->setText("Redirect not allowed");
                break;
            case ERR_UNSAFE_PORT:
                ui->errorText->setText("Disallowed Port");
                break;
            case ERR_INVALID_RESPONSE:
                ui->errorText->setText("Invalid Response");
                break;
            case ERR_UNKNOWN_URL_SCHEME:
                ui->errorText->setText("Unknown URL Scheme");
                break;
            case ERR_INVALID_URL:
                ui->errorText->setText("Invalid URL");
                break;
            case -21: //Network change detected
                ui->errorText->setText("The internet connection changed while we were trying to connect to the page.");
                break;
            default:
                ui->errorText->setText(QString::fromStdString(failedUrl.ToString()) + " may be down or it may have moved somewhere else. " );
            }
            ui->errorFrame->setVisible(true);
            browserWidget->setVisible(false);
        }
    }
}

void MainWindow::BeforeClose(Browser browser) {
    if (IsCorrectBrowser(browser)) {
        //Flush cookies (to make sure)
        CefCookieManager::GetGlobalManager(NULL).get()->FlushStore(NULL);

        //Actually close the window
        this->close();
    }
}

void MainWindow::on_JsDialogOk_clicked()
{
    ui->JsDialogFrame->setVisible(false);
    browserWidget->setVisible(true);
    JsDialogCallback.get()->Continue(true, ui->JsDialogPrompt->text().toStdString());
}

void MainWindow::on_JsDialogCancel_clicked()
{
    ui->JsDialogFrame->setVisible(false);
    browserWidget->setVisible(true);
    JsDialogCallback.get()->Continue(false, "");
}

void MainWindow::BeforeUnloadDialog(Browser browser, const CefString &message_text, bool is_reload, CefRefPtr<CefJSDialogCallback> callback) {
    if (IsCorrectBrowser(browser)) {
        JsDialogCallback = callback;
        if (is_reload) {
            ui->JsDialogOk->setText("Reload anyway");
            ui->JsDialogCancel->setText("Don't Reload");
        } else {
            ui->JsDialogOk->setText("Leave anyway");
            ui->JsDialogCancel->setText("Don't leave");
        }
        ui->JsDialogText->setText(QString::fromStdString(message_text.ToString()));
        ui->JsDialogOk->setVisible(true);
        ui->JsDialogCancel->setVisible(true);
        ui->JsDialogPrompt->setVisible(false);
        browserWidget->setVisible(false);
        ui->JsDialogFrame->setVisible(true);
    }
}

void MainWindow::on_JsDialogPrompt_returnPressed()
{
    ui->JsDialogOk->click();
}

void MainWindow::AuthCredentials(Browser browser, CefRefPtr<CefFrame> frame, bool isProxy, const CefString &host, int port, const CefString &realm, const CefString &scheme, CefRefPtr<CefAuthCallback> callback) {
    if (IsCorrectBrowser(browser)) {
        AuthCallback = callback;
        browserWidget->setVisible(false);

        ui->AuthHost->setText("Log in to " + QString::fromStdString(host.ToString()));
        ui->AuthRealm->setText("Server Realm: " + QString::fromStdString(realm.ToString()));
        ui->AuthUsername->setText("");
        ui->AuthPassword->setText("");
        if (QUrl(QString::fromStdString(frame.get()->GetURL().ToString())).scheme() == "http" && scheme == "basic") {
            ui->AuthBASIC->setVisible(true);
        } else {
            ui->AuthBASIC->setVisible(false);
        }
        ui->AuthFrame->setVisible(true);

        ui->AuthUsername->setFocus();
    }
}

void MainWindow::on_AuthOk_clicked()
{
    ui->AuthFrame->setVisible(false);
    browserWidget->setVisible(true);
    AuthCallback.get()->Continue(ui->AuthUsername->text().toStdString(), ui->AuthPassword->text().toStdString());
}

void MainWindow::on_AuthCancel_clicked()
{
    ui->AuthFrame->setVisible(false);
    browserWidget->setVisible(true);
    AuthCallback.get()->Cancel();
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
    if (IsCorrectBrowser(browser)) {
        Browser newBrowser = browser.get()->GetHost().get()->CreateBrowserSync(*windowInfo, CefRefPtr<CefHandler>(handler), target_url.ToString(), CefBrowserSettings(), CefRefPtr<CefRequestContext>());
        MainWindow* newWin = new MainWindow(newBrowser);
        newWin->show();
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
    browser.get()->GetMainFrame().get()->LoadURL("theweb://theweb");
}

void MainWindow::on_actionSettings_triggered()
{
    browser.get()->GetMainFrame().get()->LoadURL("theweb://settings");
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
        browserWidget->setVisible(true);
    }
}

void MainWindow::on_fraudBack_clicked()
{
    ui->actionGo_Back->trigger();
    ui->fraudFrame->setVisible(false);
    browserWidget->setVisible(true);
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
        certCallback = callback;

        switch (cert_error) {
        case ERR_CERT_COMMON_NAME_INVALID:
            ui->certMoreText->setText("You're trying to connect to <b>" + QUrl(QString::fromStdString(request_url.ToString())).host() + "</b> but the server presented itself as <b>" +
                                      QString::fromStdString(ssl_info.get()->GetSubject().get()->GetDisplayName().ToString()) + "</b>.");
            break;
        case ERR_CERT_DATE_INVALID:
            ui->certMoreText->setText("The server presented a certificate that either seems to not be valid yet, or has expired. <b>Check your system clock "
                                      "and if it is incorrect, set it to the correct time.</b>");
            break;
        case ERR_CERT_AUTHORITY_INVALID:
            ui->certMoreText->setText("The server presented a certificate which was signed by an authority that theWeb doesn't know about. This could mean that "
                                      "an attacker could be intercepting your connection and providing his own certificate, or it could mean that theWeb "
                                      "doesn't have this certificate authority in the database.");
            break;
        case ERR_CERT_CONTAINS_ERRORS:
        case ERR_CERT_INVALID:
            ui->certMoreText->setText("The server tried to tell theWeb who it was, but theWeb received something that didn't make sense.");
            break;
        case ERR_CERT_UNABLE_TO_CHECK_REVOCATION:
            ui->certMoreText->setText("The server sent us a certificate, but theWeb couldn't tell whether it has been revoked or not.");
            break;
        case ERR_CERT_REVOKED:
            ui->certMoreText->setText("The server sent us a certificate that has been revoked.");
            break;
        case ERR_CERT_NON_UNIQUE_NAME:
            ui->certMoreText->setText("The server sent us a certificate for a non-unique hostname.");
            break;
        default:
            ui->certMoreText->setText("The server sent us a certificate that theWeb doesn't trust.");
            break;
        }

        certErrorUrls.append(QString::fromStdString(request_url.ToString()));

        ui->securityEVName->setText("Insecure Connection!");
        ui->securityEVName->setVisible(true);
        ui->securityFrame->setStyleSheet("background-color: #640000; color: white;");
        ui->securityPadlock->setPixmap(QIcon(":/icons/badsecure").pixmap(16, 16));

        ui->securityText->setText("This connection may have been intercepted.");

        browserWidget->setVisible(false);
        ui->badCertificateFrame->setVisible(true);
    }
}

void MainWindow::on_certIgnore_clicked()
{
    if (ui->certMoreInfo->isVisible()) {
        ui->certMoreInfo->setVisible(false);
        ui->badCertificateFrame->setVisible(false);
        ui->certIgnore->setText("More Info");
        browserWidget->setVisible(true);
        certCallback.get()->Continue(true);
    } else {
        ui->certMoreInfo->setVisible(true);
        ui->certIgnore->setText("Continue Anyway");
    }
}

void MainWindow::on_certificateBack_clicked()
{
    ui->actionGo_Back->trigger();
    ui->badCertificateFrame->setVisible(false);
    browserWidget->setVisible(true);
}

void MainWindow::ReloadSettings() {
    QSettings settings;
    if (settings.value("browser/toolbarOnBottom").toBool()) {
        this->addToolBar(Qt::BottomToolBarArea, ui->toolBar);
    } else {
        this->addToolBar(Qt::TopToolBarArea, ui->toolBar);
    }
}
