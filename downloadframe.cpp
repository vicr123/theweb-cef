#include "downloadframe.h"

extern QString calculateSize(quint64 size);

DownloadFrame::DownloadFrame(CefRefPtr<CefDownloadItem> download_item, QWidget *parent) : QFrame(parent)
{
    this->setFrameShape(QFrame::StyledPanel);
    this->setFrameShadow(QFrame::Raised);
    this->downloadId = download_item.get()->GetId();

    fileNameLabel = new QLabel;
    QFont font = fileNameLabel->font();
    font.setBold(true);
    fileNameLabel->setFont(font);
    fileNameLabel->setText(QString::fromStdString(download_item.get()->GetSuggestedFileName().ToString()));

    infoLabel = new QLabel;
    infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    infoLabel->setText("Pending...");

    progressBar = new QProgressBar;
    progressBar->setMaximum(0);
    progressBar->setValue(0);

    pauseButton = new QPushButton;
    pauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    connect(pauseButton, &QPushButton::clicked, [=]() {
        if (paused) {
            cancelCallback.get()->Resume();
        } else {
            cancelCallback.get()->Pause();
        }
    });

    cancelButton = new QPushButton;
    cancelButton->setIcon(QIcon::fromTheme("media-playback-stop"));
    connect(cancelButton, &QPushButton::clicked, [=]() {
        cancelCallback.get()->Cancel();
    });

    QBoxLayout* mainLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    QBoxLayout* infoLayout = new QBoxLayout(QBoxLayout::LeftToRight);

    infoLayout->addWidget(fileNameLabel);
    infoLayout->addWidget(infoLabel);
    layout->addLayout(infoLayout);
    layout->addWidget(progressBar);

    mainLayout->addLayout(layout);
    mainLayout->addWidget(pauseButton);
    mainLayout->addWidget(cancelButton);

    this->setLayout(mainLayout);
}

void DownloadFrame::DownloadUpdated(Browser browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback) {
    if (download_item.get()->GetId() == this->downloadId) {
        file = QString::fromStdString(download_item.get()->GetFullPath().ToString());
        if (download_item.get()->IsCanceled()) {
            infoLabel->setText("Cancelled");
            progressBar->setVisible(false);
            cancelButton->setVisible(false);
            pauseButton->setVisible(false);
        } else {
            fileNameLabel->setText(QFileInfo(QString::fromStdString(download_item.get()->GetFullPath().ToString())).fileName());
            if (paused) {
                pauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
                infoLabel->setText("Paused");
            } else {
                pauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
                if (download_item.get()->IsComplete()) {
                    if (!done) {
                        done = true;
                        infoLabel->setText("Download Complete.");
                        progressBar->setMaximum(download_item.get()->GetTotalBytes());
                        progressBar->setValue(download_item.get()->GetReceivedBytes());
                        progressBar->setVisible(false);
                        cancelButton->setVisible(false);
                        pauseButton->setVisible(false);
                    }
                } else {
                    if (download_item.get()->GetPercentComplete() != -1) {
                        infoLabel->setText(calculateSize(download_item.get()->GetReceivedBytes()) + "/" + calculateSize(download_item.get()->GetTotalBytes()) + " (" + calculateSize(download_item.get()->GetCurrentSpeed()) + "/s)");
                        progressBar->setMaximum(download_item.get()->GetTotalBytes());
                        progressBar->setValue(download_item.get()->GetReceivedBytes());
                    } else {
                        infoLabel->setText("Downloading...");
                        progressBar->setMaximum(0);
                        progressBar->setValue(0);
                    }
                }
            }
            cancelCallback = callback;
        }
    }
}

void DownloadFrame::mouseReleaseEvent(QMouseEvent *event) {
    if (done) {
        QProcess::startDetached("xdg-open \"" + file + "\"");
    }
}
