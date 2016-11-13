#ifndef CERTIFICATEINFO_H
#define CERTIFICATEINFO_H

#include <QDialog>
#include <QSslCertificate>
#include <QSslCertificateExtension>
#include <QLabel>
#include <QIcon>

namespace Ui {
class CertificateInfo;
}

class CertificateInfo : public QDialog
{
    Q_OBJECT

public:
    explicit CertificateInfo(QString type, QSslCertificate certificate, QWidget *parent = 0);
    explicit CertificateInfo(QString type, QWidget *parent = 0);
    ~CertificateInfo();

private slots:
    void on_pushButton_clicked();

private:
    Ui::CertificateInfo *ui;
    QSslCertificate certificate;
};

#endif // CERTIFICATEINFO_H
