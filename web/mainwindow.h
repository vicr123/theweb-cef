#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <browsertab.h>
#include <QMenuBar>
#include <QSettings>
#include <QFrame>
#include <QBoxLayout>
#include <QStackedWidget>
#include <QLineEdit>
#include <QToolBar>
#include "bar.h"

#ifdef Q_OS_LINUX
#include <ttoast.h>
#endif

class BrowserTab;

#define CURRENT_TAB ((BrowserTab*) ui->tabs->currentWidget())

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(CefRefPtr<CefBrowser> browser = nullptr, QWidget *parent = 0);
        ~MainWindow();

        void setFullScreen(BrowserTab* tab, bool isFullScreen);

    public slots:
        void createNewTab(CefRefPtr<CefBrowser> browser = nullptr);

    private slots:
        void on_tabs_currentChanged(int arg1);

        void on_newTabButton_clicked();

        void on_closeTabButton_clicked();

        void on_actionBack_triggered();

        void on_actionForward_triggered();

        void on_actionReload_triggered();

        void on_actionNew_Tab_triggered();

        void on_actionNew_Window_triggered();

        void on_actionExit_triggered();

    private:
        Ui::MainWindow *ui;

        QSettings settings;
        Bar* addressBar;
        void closeEvent(QCloseEvent* event);

        bool wasMaximisedBeforeFullScreen;

#ifdef Q_OS_MAC
        QTabBar* tabBar;
#endif
};

#endif // MAINWINDOW_H
