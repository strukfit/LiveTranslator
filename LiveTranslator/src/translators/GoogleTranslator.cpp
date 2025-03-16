#include "translators/GoogleTranslator.h"
#include "utils/NetworkErrorHandler.h"
#include "utils/RESTApiHandler.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>

GoogleTranslator::GoogleTranslator(const QString& apiKey, QObject* parent)
	: Translator(parent),
	m_apiKey(apiKey),
	m_apiHandler(RESTApiHandler::instance())
{
}

void GoogleTranslator::translate(const QString& text, const QString& sourceLang, const QString& targetLang)
{
	QUrl url(QString("https://translation.googleapis.com/language/translate/v2?key=%1").arg(m_apiKey));

	QJsonObject json;
	json["q"] = text;
	json["source"] = sourceLang == "auto" ? "" : sourceLang;
	json["target"] = targetLang;

	QNetworkReply* reply = m_apiHandler->post(url, json);
	connect(reply, &QNetworkReply::finished, this, [=]() {
		QString errorMessage;
		if (NetworkErrorHandler::handleReplyErrors(reply, errorMessage))
		{
			emit translationError(errorMessage);
			reply->deleteLater();
			return;
		}

		QByteArray responseData = reply->readAll();
		reply->deleteLater();
		QJsonDocument doc = QJsonDocument::fromJson(responseData);
		QJsonObject responseObj = doc.object();

		if (NetworkErrorHandler::handleResponseErrors(responseObj, errorMessage))
		{
			emit translationError(errorMessage);
			return;
		}

		QString translated = doc.object()["data"]
			.toObject()["translations"]
			.toArray()[0]
			.toObject()["translatedText"]
			.toString();

		emit translationFinished(translated);
	});
}
