#include "downloadframe.h"

extern QString calculateSize(quint64 size);

DownloadFrame::DownloadFrame(CefRefPtr<CefDownloadItem> download_item, QWidget *parent) : QFrame(parent)
{
    deleteAction = new QAction;
    deleteAction->setText("Delete Download");
    deleteAction->setIcon(QIcon::fromTheme("edit-delete"));
    connect(deleteAction, &QAction::triggered, [=]() {
        if (QMessageBox::question(parent, "Delete?", "Delete this download permanantly?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
            QFile(file).remove();
            this->deleteLater();
        }
    });

    showInFolderAction = new QAction;
    showInFolderAction->setText("Show folder");
    showInFolderAction->setIcon(QIcon::fromTheme("folder"));
    connect(showInFolderAction, &QAction::triggered, [=]() {
        QProcess::startDetached("xdg-open \"" + file.left(file.lastIndexOf("/")) + "\"");
    });

    hideAction = new QAction;
    hideAction->setText("Hide Download");
    hideAction->setIcon(QIcon::fromTheme("dialog-ok"));
    connect(hideAction, &QAction::triggered, [=]() {
        this->deleteLater();
    });

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
    pauseButton->setFlat(true);
    connect(pauseButton, &QPushButton::clicked, [=]() {
        if (paused) {
            paused = false;
            cancelCallback.get()->Resume();
        } else {
            paused = true;
            cancelCallback.get()->Pause();
            pauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
            infoLabel->setText("Paused");
        }
    });

    cancelButton = new QPushButton;
    cancelButton->setIcon(QIcon::fromTheme("media-playback-stop"));
    cancelButton->setFlat(true);
    connect(cancelButton, &QPushButton::clicked, [=]() {
        cancelCallback.get()->Cancel();
    });

    QMenu* menu = new QMenu();
    menu->addAction(showInFolderAction);
    menu->addAction(deleteAction);
    menu->addAction(hideAction);

    QToolButton* menuButton = new QToolButton();
    menuButton->setArrowType(Qt::UpArrow);
    menuButton->setMenu(menu);
    menuButton->setPopupMode(QToolButton::InstantPopup);

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
    mainLayout->addWidget(menuButton);

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

            deleteAction->setVisible(false);
            hideAction->setVisible(true);
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
                        emit Completed();
                        infoLabel->setText("Download Complete.");
                        progressBar->setMaximum(download_item.get()->GetTotalBytes());
                        progressBar->setValue(download_item.get()->GetReceivedBytes());
                        progressBar->setVisible(false);
                        cancelButton->setVisible(false);
                        pauseButton->setVisible(false);

                        deleteAction->setVisible(true);
                        hideAction->setVisible(true);
                    }
                } else {
                    if (download_item.get()->GetPercentComplete() != -1) {
                        infoLabel->setText(calculateSize(download_item.get()->GetReceivedBytes()) + "/" + calculateSize(download_item.get()->GetTotalBytes()) + " (" + calculateSize(download_item.get()->GetCurrentSpeed()) + "/s)");
                        progressBar->setMaximum(download_item.get()->GetTotalBytes());
                        progressBar->setValue(download_item.get()->GetReceivedBytes());
                    } else {
                        infoLabel->setText("Downloading... (" + calculateSize(download_item.get()->GetCurrentSpeed()) + "/s)");
                        progressBar->setMaximum(0);
                        progressBar->setValue(0);
                    }
                    deleteAction->setVisible(false);
                    hideAction->setVisible(false);
                }
            }
            cancelCallback = callback;
        }
    }
}

void DownloadFrame::mouseReleaseEvent(QMouseEvent *event) {
    if (done && infoLabel->text() != "Opening...") {
        QProcess::startDetached("xdg-open \"" + file + "\"");
        infoLabel->setText("Opening...");

        QTimer* timer = new QTimer;
        timer->setSingleShot(true);
        timer->setInterval(3000);
        connect(timer, &QTimer::timeout, [=]() {
            infoLabel->setText("Download Complete.");
        });
        connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()));
        timer->start();
    }
}
