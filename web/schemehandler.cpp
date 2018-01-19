#include "schemehandler.h"
#include <QDebug>


theWebSchemeResourceHandler::theWebSchemeResourceHandler()
{

}

bool theWebSchemeResourceHandler::ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) {
    if (request.get()->GetURL() == "theweb://crash") {
        servingPage = Crash;
    } else if (request.get()->GetURL() == "theweb://kill") {
        servingPage = Kill;
    } else {
        QUrl url(QString::fromStdString(request.get()->GetURL().ToString()));

        if (url.host() == "icon") {
            servingPage = Icon;
            resourcePath = url.path().mid(1);
        } else {
            resourcePath = ":/thewebscheme/" + url.host() + url.path();


            if (!QFile(resourcePath).exists()) {
                servingPage = Invalid;
            }
        }

    }
    callback.get()->Continue();
    return true;
}

void theWebSchemeResourceHandler::GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) {
    if (servingPage == Invalid) {
        response.get()->SetError(ERR_INVALID_URL);
    } else if (servingPage == Crash) {
        redirectUrl = "chrome://crash";
        response.get()->SetStatus(301);
    } else if (servingPage == Kill) {
        redirectUrl = "chrome://kill";
        response.get()->SetStatus(301);
    } else if (servingPage == Icon) {
        if (!QIcon::hasThemeIcon(resourcePath)) {
            response.get()->SetError(ERR_INVALID_URL);
            return;
        }

        response.get()->SetMimeType("image/png");
        response.get()->SetStatus(200);
        response_length = -1;

        resourceFile = new QBuffer();
        QIcon::fromTheme(resourcePath).pixmap(32, 32).save(resourceFile, "PNG");

        resourceFile->open(QBuffer::ReadOnly);
    } else {
        resourceFile = new QFile(resourcePath);
        resourceFile->open(QFile::ReadOnly);

        if (resourcePath == ":/thewebscheme/css.css") {
            response.get()->SetMimeType("text/css");
        } else {
            response.get()->SetMimeType(mimeDb.mimeTypeForData(resourceFile).name().toStdString());
        }

        resourceFile->seek(0);

        response.get()->SetStatus(200);
        response_length = -1;
    }
}

void theWebSchemeResourceHandler::Cancel() {

}

bool theWebSchemeResourceHandler::ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) {
    if (!resourceFile->isOpen()) {
        return false;
    }

    QByteArray buf = resourceFile->read(bytes_to_read);
    memcpy(data_out, (const char*) buf.data(), buf.size());
    bytes_read = buf.size();

    if (resourceFile->atEnd()) {
        resourceFile->close();
        resourceFile->deleteLater();
    }

    return true;
}
