#ifndef NETWORKERRORHANDLER_H
#define NETWORKERRORHANDLER_H

#include <QMap>
#include <QJsonObject>


class QNetworkReply;

class NetworkErrorHandler {
public:
    static bool handleReplyErrors(QNetworkReply* reply, QString& errorMessage);
    static bool handleResponseErrors(const QJsonObject& responseObj, QString& errorMessage);

private:
    inline static const QMap<QString, QString> m_errorMessagesMap = {
        {"REQUEST_ERROR", "Error during request execution..."},
        {"INVALID_API_KEY", "The provided API key is invalid."},
        {"QUOTA_EXCEEDED", "API quota exceeded..."},
        {"INVALID_REQUEST", "Invalid request format..."},
        {"RESOURCE_NOT_FOUND", "The requested resource was not found..."},
        {"SERVER_ERROR", "Server error occurred..."},
        {"UNKNOWN_ERROR", "An unknown error occurred..."}
    };

    inline static const QMap<QString, QString> m_errorCodeAliases = {
        {"400", "INVALID_REQUEST"},
        {"invalidParameter", "INVALID_REQUEST"},
        {"401", "INVALID_API_KEY"},
        {"invalidApiKey", "INVALID_API_KEY"},
        {"403", "QUOTA_EXCEEDED"},
        {"quotaExceeded", "QUOTA_EXCEEDED"},
        {"404", "RESOURCE_NOT_FOUND"},
        {"500", "SERVER_ERROR"},
        {"503", "SERVER_ERROR"},
        {"REQUEST_ERROR", "REQUEST_ERROR"},
        {"UNKNOWN_ERROR", "UNKNOWN_ERROR"}
    };

    static QString getErrorMessage(const QString& errorCode);
};

#endif // NETWORKERRORHANDLER_H