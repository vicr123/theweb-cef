#include "browserdialog.h"
#include "ui_browserdialog.h"

BrowserDialog::BrowserDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BrowserDialog)
{
    ui->setupUi(this);

    ui->jsDialogFrame->installEventFilter(this);
    ui->AuthDialogFrame->installEventFilter(this);
    ui->certErrorFrame->setFixedWidth(700);
}

BrowserDialog::~BrowserDialog()
{
    delete ui;
}

void BrowserDialog::alert(QString message, CefRefPtr<CefJSDialogCallback> callback) {
    ui->jsDialogTitle->setText(tr("Alert"));
    ui->jsDialogMessage->setText(message);
    ui->jsDialogCancel->setVisible(false);
    ui->jsPromptEdit->setVisible(false);
    this->jsCallback = callback;
    ui->stackedWidget->setCurrentIndex(0);
    ui->jsDialogOk->setText(tr("OK"));

    activate();
}

void BrowserDialog::confirm(QString message, CefRefPtr<CefJSDialogCallback> callback) {
    ui->jsDialogTitle->setText(tr("Confirm"));
    ui->jsDialogMessage->setText(message);
    ui->jsDialogCancel->setVisible(true);
    ui->jsPromptEdit->setVisible(false);
    this->jsCallback = callback;
    ui->stackedWidget->setCurrentIndex(0);
    ui->jsDialogOk->setText(tr("OK"));
    ui->jsDialogCancel->setText(tr("Cancel"));

    activate();
}

void BrowserDialog::beforeUnload(bool isReload, CefRefPtr<CefJSDialogCallback> callback) {
    if (isReload) {
        ui->jsDialogTitle->setText(tr("Reload?"));
        ui->jsDialogOk->setText(tr("Reload anyway"));
        ui->jsDialogCancel->setText(tr("Don't reload"));
    } else {
        ui->jsDialogTitle->setText(tr("Leave?"));
        ui->jsDialogOk->setText(tr("Leave anyway"));
        ui->jsDialogCancel->setText(tr("Don't leave"));
    }

    ui->jsDialogMessage->setText(tr("You might lose data if you leave this page."));
    ui->jsPromptEdit->setVisible(false);
    this->jsCallback = callback;
    ui->stackedWidget->setCurrentIndex(0);

    activate();
}

void BrowserDialog::prompt(QString message, QString def, CefRefPtr<CefJSDialogCallback> callback) {
    ui->jsDialogTitle->setText(tr("Prompt"));
    ui->jsDialogMessage->setText(message);
    ui->jsDialogCancel->setVisible(true);
    ui->jsPromptEdit->setVisible(true);
    ui->jsPromptEdit->setText(def);
    this->jsCallback = callback;
    ui->stackedWidget->setCurrentIndex(0);
    ui->jsDialogOk->setText(tr("OK"));
    ui->jsDialogCancel->setText(tr("Cancel"));

    activate();

}

void BrowserDialog::on_jsDialogOk_clicked()
{
    jsCallback.get()->Continue(true, ui->jsPromptEdit->text().toStdString());

    dismiss();
}

void BrowserDialog::on_jsDialogCancel_clicked()
{
    jsCallback.get()->Continue(false, "");

    dismiss();
}

bool BrowserDialog::eventFilter(QObject *watched, QEvent *event) {
    if ((watched == ui->jsDialogFrame || watched == ui->AuthDialogFrame) && event->type() == QEvent::Paint) {
        QWidget* w = (QWidget*) watched;
        QPainter painter(w);

        painter.setPen(Qt::transparent);
        painter.setBrush(w->palette().brush(QPalette::Window));
        painter.drawRect(0, 0, w->width(), w->height());

        painter.setPen(w->palette().color(QPalette::WindowText));
        painter.drawLine(0, 0, w->width(), 0);
        return true;
    }
    return false;
}

void BrowserDialog::on_reloadButton_clicked()
{
    ((BrowserTab*) this->parent())->reload();
}

void BrowserDialog::reset() {
    if (jsCallback.get() != nullptr) {
        jsCallback.get()->Continue(false, "");
    }
    dismiss();
}

void BrowserDialog::authenticate(QString host, QString realm, CefRefPtr<CefAuthCallback> callback) {
    ui->httpAuthHost->setText(tr("Log in to %1").arg(host));
    ui->httpAuthRealm->setText(realm);
    this->authCallback = callback;
    ui->stackedWidget->setCurrentIndex(1);
    ui->httpAuthUsername->setText("");
    ui->httpAuthPassword->setText("");

    activate();
}

void BrowserDialog::on_httpAuthCancel_clicked()
{
    authCallback.get()->Cancel();
    dismiss();
}

void BrowserDialog::on_httpAuthOk_clicked()
{
    authCallback.get()->Continue(ui->httpAuthUsername->text().toStdString(), ui->httpAuthPassword->text().toStdString());
    dismiss();
}

void BrowserDialog::certificate(QString extraInformation, bool isHSTS, CefRefPtr<CefRequestCallback> callback) {
    ui->stackedWidget->setCurrentIndex(2);
    ui->certErrorExtra->setText(extraInformation);
    ui->certErrorExtra->setVisible(false);
    ui->certErrorAction->setText(tr("Go Back"));
    ui->certErrorAction->setIcon(QIcon::fromTheme("go-previous"));
    ui->certErrorAction->setProperty("type", "positive");
    this->reqCallback = callback;

    activate();
}
void BrowserDialog::on_certErrorAction_clicked()
{
    if (ui->certErrorExtra->isVisible()) {
        this->reqCallback.get()->Continue(true);
    } else {
        this->reqCallback.get()->Continue(false);
    }

    dismiss();
}

void BrowserDialog::on_certErrorMore_clicked()
{
    if (ui->certErrorExtra->isVisible()) {
        ui->certErrorExtra->setVisible(false);
        ui->certErrorAction->setText(tr("Go Back"));
        ui->certErrorAction->setIcon(QIcon::fromTheme("go-previous"));
        ui->certErrorAction->setProperty("type", "positive");
    } else {
        ui->certErrorExtra->setVisible(true);
        ui->certErrorAction->setText(tr("Continue Anyway"));
        ui->certErrorAction->setIcon(QIcon::fromTheme("go-next"));
        ui->certErrorAction->setProperty("type", "destructive");
    }
}
