#include "tabbutton.h"

extern float getDPIScaling();

TabButton::TabButton(QWidget* parent) : QPushButton(parent)
{
    animationTimer = new QTimer();
    animationTimer->setInterval(10);
    connect(animationTimer, &QTimer::timeout, [=] {
        animationCounter += (this->width() / 50) + 1;
        this->update();
    });

    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
}

void TabButton::setLoading(bool isLoading) {
    this->isLoading = isLoading;
    this->update();

    if (isLoading) {
        animationTimer->start();
    } else {
        animationTimer->stop();
    }
}

void TabButton::enterEvent(QEvent *event) {
    hovering = true;
}

void TabButton::leaveEvent(QEvent *event) {
    hovering = false;
}

void TabButton::setSiteIcon(QStringList urls) {
    downloadUrl(urls, 0);
}

void TabButton::downloadUrl(QStringList urls, int url) {
    if (url >= urls.length()) {
        this->setIcon(QIcon::fromTheme("document-new"));
        return;
    }

    QNetworkRequest req;
    req.setUrl(QUrl(urls.at(url)));
    QNetworkReply* reply = mgr.get(req);
    connect(reply, &QNetworkReply::finished, [=] {
        QPixmap pixmap;
        pixmap.loadFromData(reply->readAll());

        reply->deleteLater();

        if (pixmap.isNull()) {
            downloadUrl(urls, url + 1);
            return;
        }

        QIcon icon(pixmap);
        this->setIcon(icon);
    });
}

void TabButton::paintEvent(QPaintEvent *event) {
    QPalette pal = this->palette();
    QPainter painter(this);
    QBrush brush;
    QPen textPen;

    QRect rect = QRect(0, 0, this->width(), this->height());

    if (this->isEnabled()) {
        brush = QBrush(pal.color(QPalette::Button));
    } else {
        brush = QBrush(pal.color(QPalette::Disabled, QPalette::Button));
    }

    if (this->hasFocus()) {
        brush = QBrush(pal.color(QPalette::Button).lighter(125));
    }

    if (hovering) {
        brush = QBrush(pal.color(QPalette::Button).lighter());
    }

    if (this->isDown() || this->isChecked()) {
        brush = QBrush(pal.color(QPalette::Button).darker(150));
    }
    textPen = pal.color(QPalette::ButtonText);

    painter.setBrush(brush);
    painter.setPen(Qt::transparent);
    painter.drawRect(rect);

    QRect textRect, iconRect;
    textRect.setWidth(this->fontMetrics().width(this->text()));
    textRect.moveTop(rect.top() + this->height() / 2 - this->fontMetrics().height() / 2);
    textRect.setHeight(this->fontMetrics().height());

    int left;
    left = rect.left() + this->width() / 2 - textRect.width() / 2;
    if (left < rect.left() + 8 * getDPIScaling()) {
        left = rect.left() + 8 * getDPIScaling();
    }
    textRect.moveLeft(left);

    if (!this->icon().isNull()) {
        int iconLeft = textRect.left() - 16 - 4 * getDPIScaling();
        if (iconLeft < rect.left() + 8 * getDPIScaling()) {
            iconLeft = rect.left() + 8 * getDPIScaling();
        }

        iconRect.setLeft(iconLeft);
        iconRect.setTop(rect.top() + (rect.height() / 2) - (this->iconSize().height() / 2));
        iconRect.setSize(this->iconSize());

        textRect.moveLeft(iconRect.right() + 4 * getDPIScaling());
        //oldTextRect.moveLeft(iconRect.right() + 4 * getDPIScaling());

        QIcon icon = this->icon();
        QImage image = icon.pixmap(this->iconSize()).toImage();
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

        painter.drawImage(iconRect, image);
    }

    //Draw text
    painter.setPen(textPen);
    painter.setBrush(Qt::transparent);
    //painter.drawText(oldTextRect, Qt::AlignLeft, oldText.remove("&"));
    painter.drawText(textRect, Qt::AlignLeft, this->text());

    QLinearGradient endGrad;
    endGrad.setStart(this->width() - 25 * getDPIScaling(), 0);
    endGrad.setFinalStop(this->width(), 0);
    endGrad.setColorAt(0, Qt::transparent);
    endGrad.setColorAt(1, brush.color());
    painter.setBrush(endGrad);
    painter.setPen(Qt::transparent);
    painter.drawRect(rect);

    if (isLoading) {
        QRect rect;
        rect.setTop(this->height() - 4);
        rect.setLeft(-this->width() + animationCounter % (this->width() * 2));
        rect.setHeight(4);
        rect.setWidth(this->width());

        QPainter painter(this);
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(0, 150, 255));
        painter.drawRect(rect);
    }
}

void TabButton::setText(QString text) {

    QPushButton::setText(text);
    emit textChange(text);
}
