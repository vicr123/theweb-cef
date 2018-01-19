#ifndef BROWSERDIALOG_H
#define BROWSERDIALOG_H

#include <QWidget>
#include <include/cef_jsdialog_handler.h>
#include <include/cef_load_handler.h>
#include <QLineEdit>
#include <QPaintEvent>
#include <QPainter>
#include <QStackedWidget>
#include "browsertab.h"

class BrowserTab;

namespace Ui {
    class BrowserDialog;
}

class BrowserDialog : public QWidget
{
        Q_OBJECT

    public:
        explicit BrowserDialog(QWidget *parent = 0);
        ~BrowserDialog();

    public slots:
        void alert(QString message, CefRefPtr<CefJSDialogCallback> callback);
        void confirm(QString message, CefRefPtr<CefJSDialogCallback> callback);
        void prompt(QString message, QString def, CefRefPtr<CefJSDialogCallback> callback);
        void beforeUnload(bool isReload, CefRefPtr<CefJSDialogCallback> callback);
        void authenticate(QString host, QString realm, CefRefPtr<CefAuthCallback> callback);
        void certificate(QString extraInformation, bool isHSTS, CefRefPtr<CefRequestCallback> callback);
        void reset();

    signals:
        void dismiss();
        void activate();

    private slots:
        void on_jsDialogOk_clicked();

        void on_jsDialogCancel_clicked();

        void on_reloadButton_clicked();

        void on_httpAuthCancel_clicked();

        void on_httpAuthOk_clicked();

        void on_certErrorAction_clicked();

        void on_certErrorMore_clicked();

    private:
        Ui::BrowserDialog *ui;

        bool eventFilter(QObject *watched, QEvent *event);

        CefRefPtr<CefJSDialogCallback> jsCallback;
        CefRefPtr<CefAuthCallback> authCallback;
        CefRefPtr<CefRequestCallback> reqCallback;
};

#endif // BROWSERDIALOG_H
