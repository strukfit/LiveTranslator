#include "utils/RESTApiHandler.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>

RESTApiHandler* RESTApiHandler::m_instance = nullptr;

RESTApiHandler* RESTApiHandler::instance()
{
	if (!m_instance)
	{
		m_instance = new RESTApiHandler();
	}
	return m_instance;
}

QNetworkReply* RESTApiHandler::get(const QUrl& url)
{
	QNetworkRequest request(url);
	return m_networkAccessManager->get(request);
}

QNetworkReply* RESTApiHandler::post(const QUrl& url, const QJsonObject& data)
{
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	return m_networkAccessManager->post(request, QJsonDocument(data).toJson());
}

QNetworkReply* RESTApiHandler::put(const QUrl& url, const QJsonObject& data)
{
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	return m_networkAccessManager->put(request, QJsonDocument(data).toJson());
}

QNetworkReply* RESTApiHandler::patch(const QUrl& url, const QJsonObject& data)
{
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	return m_networkAccessManager->sendCustomRequest(request, "PATCH", QJsonDocument(data).toJson());
}

QNetworkReply* RESTApiHandler::deleteResource(const QUrl& url)
{
	QNetworkRequest request(url);
	return m_networkAccessManager->deleteResource(request);
}

RESTApiHandler::RESTApiHandler(QObject* parent)
	: QObject(parent),
	m_networkAccessManager(new QNetworkAccessManager(this))
{
}

RESTApiHandler::~RESTApiHandler() {
	m_networkAccessManager->deleteLater();
}
