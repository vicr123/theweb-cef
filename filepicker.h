#ifndef FILEPICKER_H
#define FILEPICKER_H

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QMimeDatabase>
#include <QWindow>
#include "cefheaders.h"

class FilePicker : public QWidget
{
    Q_OBJECT
public:
    explicit FilePicker(QWidget *parent = 0);

    enum selectionType {
        single,
        multiple,
        singleFolder
    };

signals:
    void fileDone();

public slots:
    void startSelectFile(CefRefPtr<CefFileDialogCallback> callback, selectionType = single);

private slots:
    void reloadFiles();

private:
    QListWidget* fileList;
    QLabel *pathLabel, *titleLabel;
    QPushButton *okButton;
    QDir currentDir;
    QMimeDatabase mimeDatabase;
    selectionType currentType;
    CefRefPtr<CefFileDialogCallback> callback;
};

#endif // FILEPICKER_H
