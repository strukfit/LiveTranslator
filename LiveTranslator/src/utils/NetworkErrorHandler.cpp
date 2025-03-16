#include "utils/NetworkErrorHandler.h"
#include <QNetworkReply>
#include <QJsonDocument>

bool NetworkErrorHandler::handleReplyErrors(QNetworkReply* reply, QString& errorMessage)
{
    if (!reply) {
        errorMessage = getErrorMessage("REQUEST_ERROR");
        qDebug() << "No reply provided";
        return true;
    }

    if (reply->error() != QNetworkReply::NoError) {
        QByteArray serverResponse = reply->readAll();
        qDebug() << "Network error: " << reply->errorString();;

        if (!serverResponse.isEmpty() && serverResponse.startsWith('{'))
        {
            QJsonDocument doc = QJsonDocument::fromJson(serverResponse);
            if (!doc.isNull() && doc.isObject())
            {
                return handleResponseErrors(doc.object(), errorMessage);
            }
        }

        errorMessage = getErrorMessage("REQUEST_ERROR");
        errorMessage += " " + reply->errorString();
        if (!serverResponse.isEmpty()) {
            errorMessage += " Server replied: " + QString(serverResponse);
        }

        return true;
    }

    return false;
}

bool NetworkErrorHandler::handleResponseErrors(const QJsonObject& responseObj, QString& errorMessage)
{
    if (responseObj.contains("error")) {
        QJsonObject errorObj = responseObj["error"].toObject();
        qDebug() << errorObj["code"].toInt();
        QString errorCode = errorObj.contains("code") ? errorObj["code"].toVariant().toString() : "UNKNOWN_ERROR";
        QString errorDetail = errorObj.contains("message") ? errorObj["message"].toString() : "";

        qDebug() << "API error:" << errorCode << "-" << errorDetail;

        errorMessage = getErrorMessage(errorCode);
        if (!errorDetail.isEmpty()) {
            errorMessage += " Details: " + errorDetail;
        }
        return true;
    }
    return false;
}

QString NetworkErrorHandler::getErrorMessage(const QString& errorCode)
{
	QString normalizedCode = m_errorCodeAliases.contains(errorCode) ? m_errorCodeAliases[errorCode] : "UNKNOWN_ERROR";
	return m_errorMessagesMap[normalizedCode];
}
