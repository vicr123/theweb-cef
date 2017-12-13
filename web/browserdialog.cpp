#include "browserdialog.h"
#include "ui_browserdialog.h"

BrowserDialog::BrowserDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BrowserDialog)
{
    ui->setupUi(this);

    ui->jsDialogFrame->installEventFilter(this);
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
    if (watched == ui->jsDialogFrame && event->type() == QEvent::Paint) {
        QPainter painter(ui->jsDialogFrame);

        painter.setPen(Qt::transparent);
        painter.setBrush(ui->jsDialogFrame->palette().brush(QPalette::Window));
        painter.drawRect(0, 0, ui->jsDialogFrame->width(), ui->jsDialogFrame->height());

        painter.setPen(ui->jsDialogFrame->palette().color(QPalette::WindowText));
        painter.drawLine(0, 0, ui->jsDialogFrame->width(), 0);
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
