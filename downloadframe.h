#ifndef DOWNLOADFRAME_H
#define DOWNLOADFRAME_H

#include "cefheaders.h"
#include <QFrame>
#include <QProgressBar>
#include <QLabel>
#include <QBoxLayout>
#include <QFileInfo>
#include <QPushButton>
#include <QMouseEvent>
#include <QProcess>

class DownloadFrame : public QFrame
{
    Q_OBJECT
public:
    explicit DownloadFrame(CefRefPtr<CefDownloadItem> download_item, QWidget *parent = 0);

signals:

public slots:
    void DownloadUpdated(Browser browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback);

private:
    QLabel* fileNameLabel, *infoLabel;
    QProgressBar* progressBar;
    QPushButton* pauseButton, *cancelButton;

    uint32 downloadId;
    CefRefPtr<CefDownloadItemCallback> cancelCallback;

    bool done = false, paused = false;
    QString file;

    void mouseReleaseEvent(QMouseEvent* event);
};

#endif // DOWNLOADFRAME_H
