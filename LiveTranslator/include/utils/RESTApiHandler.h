#ifndef RESTAPIHANDLER_H
#define RESTAPIHANDLER_H

#include <QObject>
#include <QJsonObject>
#include <QUrl>

class QNetworkReply;
class QNetworkAccessManager;

class RESTApiHandler : public QObject 
{
    Q_OBJECT
public:
    static RESTApiHandler* instance();
    QNetworkReply* get(const QUrl& url);
    QNetworkReply* post(const QUrl& url, const QJsonObject& data);
    QNetworkReply* put(const QUrl& url, const QJsonObject& data);
    QNetworkReply* patch(const QUrl& url, const QJsonObject& data);
    QNetworkReply* deleteResource(const QUrl& url);

private:
    RESTApiHandler(QObject* parent = nullptr);
    ~RESTApiHandler() override;
    RESTApiHandler(const RESTApiHandler&) = delete;
    RESTApiHandler& operator=(const RESTApiHandler&) = delete;

    static RESTApiHandler* m_instance;
    QNetworkAccessManager* m_networkAccessManager;
};

#endif // RESTAPIHANDLER_H