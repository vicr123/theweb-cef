#include "thewebschemes.h"

theWebSchemeResourceHandler::theWebSchemeResourceHandler()
{

}

bool theWebSchemeResourceHandler::ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) {
    QUrl url(QString::fromStdString(request.get()->GetURL().ToString()));

    if (url.path() == "/css.css") {
        resourcePath = ":/thewebscheme/css.css";
    } else {
        resourcePath = ":/thewebscheme/" + url.host() + url.path();
    }

    if (!QFile(resourcePath).exists()) {
        servingPage = Invalid;
    }

    callback.get()->Continue();
    return true;
}

void theWebSchemeResourceHandler::GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) {
    if (servingPage == Invalid) {
        response.get()->SetError(ERR_INVALID_URL);
    } else {
        resourceFile.setFileName(resourcePath);
        resourceFile.open(QFile::ReadOnly);

        if (resourcePath == ":/thewebscheme/css.css") {
            response.get()->SetMimeType("text/css");
        } else {
            response.get()->SetMimeType(mimeDb.mimeTypeForData(&resourceFile).name().toStdString());
        }

        resourceFile.seek(0);

        response.get()->SetStatus(200);
        response_length = -1;
    }
}

void theWebSchemeResourceHandler::Cancel() {

}

bool theWebSchemeResourceHandler::ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) {
    if (!resourceFile.isOpen()) {
        return false;
    }

    QByteArray buf = resourceFile.read(bytes_to_read);
    memcpy(data_out, (const char*) buf.data(), buf.size());
    bytes_read = buf.size();

    if (resourceFile.atEnd()) {
        resourceFile.close();
    }

    return true;

}
