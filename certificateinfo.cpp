#include "certificateinfo.h"
#include "ui_certificateinfo.h"

CertificateInfo::CertificateInfo(QString type, QSslCertificate certificate, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertificateInfo)
{
    ui->setupUi(this);

    this->certificate = certificate;

    /*if (securityMetadata.at(0) == "certerr") {
        ui->securityFrame->setStyleSheet("background-color: #640000; color: white;");
        ui->securityPadlock->setPixmap(QIcon(":/icons/badsecure").pixmap(16, 16));
    } else {
        ui->securityPadlock->setPixmap(QIcon::fromTheme("text-html").pixmap(16, 16));
        ui->securityFrame->setStyleSheet("");
    }*/

    if (type == "ev") {
        ui->verifiedFrame->setStyleSheet("background-color: #006400; color: white;");
        ui->verifiedFrame->setVisible(true);
        ui->verifiedTick->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(16, 16));

        ui->securitySummary->setStyleSheet("background-color: #006400; color: white;");
        ui->securityPadlock->setPixmap(QIcon(":/icons/lock-d").pixmap(16, 16));

        ui->securityDescription->setText("Your connection to <b>" + certificate.subjectInfo(QSslCertificate::Organization).join(", ") + "</b> was encrypted, and the company has been verified by <b>" + certificate.issuerInfo(QSslCertificate::CommonName).join(", ") + "</b>.");
    } else if (type == "secure") {
        QColor panelColor = ui->securitySummary->palette().color(QPalette::Window);
        if (((qreal) panelColor.red() + (qreal) panelColor.green() + (qreal) panelColor.red()) / (qreal) 3 < 127) {
            ui->securityPadlock->setPixmap(QIcon(":/icons/lock-d").pixmap(16, 16));
        } else {
            ui->securityPadlock->setPixmap(QIcon(":/icons/lock-l").pixmap(16, 16));
        }

        ui->securityDescription->setText("Your connection was encrypted.");
        ui->verifiedFrame->setVisible(false);
    } else {
        ui->verifiedFrame->setVisible(false);
    }

    //Fill Subject Info fields
    ui->subjectCommonName->setText(certificate.subjectInfo(QSslCertificate::CommonName).join(", "));
    ui->subjectOrganisation->setText(certificate.subjectInfo(QSslCertificate::Organization).join(", "));
    ui->subjectOrganisationalUnit->setText(certificate.subjectInfo(QSslCertificate::OrganizationalUnitName).join(", "));

    ui->issuerCommonName->setText(certificate.issuerInfo(QSslCertificate::CommonName).join(", "));
    ui->issuerOrganisation->setText(certificate.issuerInfo(QSslCertificate::Organization).join(", "));
    ui->issuerOrganisationalUnit->setText(certificate.issuerInfo(QSslCertificate::OrganizationalUnitName).join(", "));

    ui->expiryStart->setText(certificate.effectiveDate().toString("dddd, d MMMM yyyy, hh:mm:ss"));
    ui->expiryEnd->setText(certificate.expiryDate().toString("dddd, d MMMM yyyy, hh:mm:ss"));

    ui->sha256->setText(certificate.digest(QCryptographicHash::Sha256).toHex().toUpper());
    ui->sha1->setText(certificate.digest(QCryptographicHash::Sha1).toHex().toUpper());
}

CertificateInfo::CertificateInfo(QString type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertificateInfo)
{
    ui->setupUi(this);

    ui->certInfoFrame->setVisible(false);
    if (type == "theweb") {
        ui->summary->setText("This page is generated by theWeb.");
    } else if (type == "secure") {
        ui->summary->setText("This site is secure.");
    } else if (type == "unsecure") {
        ui->summary->setText("This site has no special security. Anyone can read what you send this website.");
    } else if (type == "ev") {
        ui->summary->setText("This site uses an extended validation certificate.");
    } else {
        ui->summary->setText("Missing description: " + type);
    }
}

CertificateInfo::~CertificateInfo()
{
    delete ui;
}

void CertificateInfo::on_pushButton_clicked()
{
    this->close();
}
