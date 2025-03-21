#include "translators/DeepLTranslator.h"
#include "utils/NetworkErrorHandler.h"
#include "utils/RESTApiHandler.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>


DeepLTranslator::DeepLTranslator(const QString& apiKey, bool isApiFree, QObject* parent)
	: Translator(parent),
	m_apiKey(apiKey),
	m_apiEndpoint(QString("https://api%1.deepl.com/v2/translate").arg(isApiFree ? "-free" : ""))
{
}

void DeepLTranslator::translate(const QString& text, const QString& sourceLang, const QString& targetLang)
{
	if (sourceLang == targetLang)
	{
		return;
	}

	QUrl url(QString("%1?auth_key=%2").arg(m_apiEndpoint).arg(m_apiKey));

	QJsonObject json;
	QJsonArray textArray;
	textArray.append(text);
	json["text"] = textArray;
	json["target_lang"] = targetLang.toUpper();
	if (sourceLang != "auto")
	{
		json["source_lang"] = sourceLang.toUpper();
	}

	QNetworkReply* reply = RESTApiHandler::instance()->post(url, json);
	connect(reply, &QNetworkReply::finished, this, [=] {
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

		QString translated = doc.object()["translations"]
			.toArray()[0]
			.toObject()["text"]
			.toString();

		emit translationFinished(translated);
	});
}
