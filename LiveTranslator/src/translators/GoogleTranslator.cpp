#include "translators/GoogleTranslator.h"
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
	QUrl url("https://translation.googleapis.com/language/translate/v2");

	QJsonObject json;
	json["q"] = text;
	json["source"] = sourceLang == "auto" ? "" : sourceLang;
	json["target"] = targetLang;
	json["key"] = m_apiKey;

	QNetworkReply* reply = m_apiHandler->post(url, json);
	connect(reply, &QNetworkReply::finished, this, [=]() {
		if (reply->error() != QNetworkReply::NoError)
		{
			emit translationError(reply->errorString());
			return;
		}

		QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
		QString translated = doc.object()["data"]
			.toObject()["translations"]
			.toArray()[0]
			.toObject()["translatedText"]
			.toString();

		emit translationFinished(translated);
	});
}
