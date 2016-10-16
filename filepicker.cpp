#include "filepicker.h"

FilePicker::FilePicker(QWidget *parent) : QWidget(parent)
{
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    this->setLayout(layout);

    titleLabel = new QLabel;
    titleLabel->setText("Select a file");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(15);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);

    pathLabel = new QLabel;
    layout->addWidget(pathLabel);

    fileList = new QListWidget;
    fileList->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    connect(fileList, &QListWidget::itemActivated, [=](QListWidgetItem* item) {
        QString filename = item->data(Qt::UserRole).toString();
        if (QDir(filename).exists()) {
            currentDir = QDir(filename);
            reloadFiles();
        }
    });
    connect(fileList, &QListWidget::itemSelectionChanged, [=]() {
        if (currentType == singleFolder) {
            okButton->setEnabled(true);
        } else {
            if (fileList->selectedItems().count() == 0) {
                okButton->setEnabled(false);
            } else {
                okButton->setEnabled(true);
            }
        }
    });
    layout->addWidget(fileList);

    QBoxLayout* buttonLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    QPushButton* upButton = new QPushButton;
    upButton->setText("Go Up");
    upButton->setIcon(QIcon::fromTheme("go-parent-folder"));
    connect(upButton, &QPushButton::clicked, [=]() {
        currentDir.cdUp();
        reloadFiles();
    });
    buttonLayout->addWidget(upButton);

    buttonLayout->addStretch();

    okButton = new QPushButton;
    okButton->setText("OK");
    okButton->setIcon(QIcon::fromTheme("dialog-ok"));
    connect(okButton, &QPushButton::clicked, [=]() {
        std::vector<CefString> selectedFiles;
        if (currentType == single) {
            selectedFiles.push_back(fileList->selectedItems().first()->data(Qt::UserRole).toString().toStdString());
        } else if (currentType == multiple) {
            for (QListWidgetItem* item : fileList->selectedItems()) {
                selectedFiles.push_back(item->data(Qt::UserRole).toString().toStdString());
            }
        } else if (currentType == singleFolder) {
            selectedFiles.push_back(currentDir.path().toStdString());
        }
        callback.get()->Continue(0, selectedFiles);
        emit fileDone();
    });
    buttonLayout->addWidget(okButton);

    QPushButton* cancelButton = new QPushButton;
    cancelButton->setText("Cancel");
    cancelButton->setIcon(QIcon::fromTheme("dialog-cancel"));
    connect(cancelButton, &QPushButton::clicked, [=]() {
        callback.get()->Cancel();
        emit fileDone();
    });
    connect(cancelButton, SIGNAL(clicked(bool)), this, SIGNAL(fileDone()));
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
}

void FilePicker::startSelectFile(CefRefPtr<CefFileDialogCallback> callback, selectionType type) {
    if (type == multiple) {
        titleLabel->setText("Select some files");
        fileList->setSelectionMode(QListWidget::ExtendedSelection);
        okButton->setText("OK");
    } else if (type == singleFolder) {
        titleLabel->setText("Select a folder");
        fileList->setSelectionMode(QListWidget::SingleSelection);
        okButton->setText("Select this folder");
    } else if (type == single) {
        titleLabel->setText("Select a file");
        fileList->setSelectionMode(QListWidget::SingleSelection);
        okButton->setText("OK");
    }
    currentType = type;
    this->callback = callback;

    currentDir.setPath(QDir::homePath());
    reloadFiles();
}

void FilePicker::reloadFiles() {
    fileList->clear();
    pathLabel->setText(currentDir.path());
    if (currentDir.exists()) {
        //TODO: Replace "false" with a "Show Hidden Files" checkbox
        QFileInfoList info = currentDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | (false ? QDir::Hidden : (QDir::Filters) 0), QDir::Name | QDir::DirsFirst | QDir::IgnoreCase);

        if (info.count() == 0) {
            //showInfoMessage("There's nothing in this folder.");
        } else {
            for (int i = 0; i < info.count(); i++) {
                QListWidgetItem* fileItem = new QListWidgetItem();
                if (info[i].fileName().startsWith(".")) {
                    fileItem->setText(info[i].fileName());
                } else {
                    fileItem->setText(info[i].baseName());
                }
                fileItem->setData(Qt::UserRole, info[i].filePath());
                fileItem->setData(Qt::UserRole + 1, info[i].fileName());
                if (info[i].isDir()) {
                    if (info[i].path() == QDir::homePath() && info[i].fileName() == "Documents") {
                        fileItem->setIcon(QIcon::fromTheme("folder-documents"));
                    } else if (info[i].path() == QDir::homePath() && info[i].fileName() == "Downloads") {
                            fileItem->setIcon(QIcon::fromTheme("folder-downloads"));
                    } else if (info[i].path() == QDir::homePath() && info[i].fileName() == "Music") {
                            fileItem->setIcon(QIcon::fromTheme("folder-music"));
                    } else if (info[i].path() == QDir::homePath() && info[i].fileName() == "Videos") {
                            fileItem->setIcon(QIcon::fromTheme("folder-videos"));
                    } else if (info[i].path() == QDir::homePath() && info[i].fileName() == "Pictures") {
                            fileItem->setIcon(QIcon::fromTheme("folder-pictures"));
                    } else {
                        fileItem->setIcon(QIcon::fromTheme("folder"));
                    }
                } else {
                    if (currentType == singleFolder) {
                        delete fileItem;
                        continue;
                    } else {
                        fileItem->setIcon(QIcon::fromTheme(mimeDatabase.mimeTypeForFile(info[i].filePath()).iconName()));
                    }
                }

                if (info[i].isHidden()) {
                    QBrush disabledColor = fileList->palette().brush(QPalette::Disabled, QPalette::Foreground);
                    fileItem->setForeground(disabledColor);
                }

                fileList->addItem(fileItem);
            }
        }
    }
}
