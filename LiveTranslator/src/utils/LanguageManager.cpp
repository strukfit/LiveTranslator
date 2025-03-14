#include "utils/LanguageManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

LanguageManager::LanguageManager(const QString& jsonFilePath)
{
	loadFromJson(jsonFilePath);

	if (languages.isEmpty())
	{
		qWarning() << "Failed to load languages from JSON, using defaults";
		addDefaultLanguages();
	}
}

void LanguageManager::addLanguage(const QString& displayName, const QString& ocrCode, const QString& apiCode)
{
	languages[displayName] = { ocrCode, apiCode };
}

QString LanguageManager::getOcrCode(const QString& displayName) const
{
	auto it = languages.find(displayName);
	if (it != languages.end())
	{
		return it.value().ocrCode;
	}

	qWarning() << "Language not found:" << displayName << ", returning default 'eng'";
	return "eng";
}

QString LanguageManager::getApiCode(const QString& displayName) const
{
	auto it = languages.find(displayName);
	if (it != languages.end()) 
	{
		return it.value().apiCode;
	}

	qWarning() << "Language not found:" << displayName << ", returning default 'en'";
	return "en";
}

QStringList LanguageManager::getLanguagesDisplayNames() const
{
	return languages.keys();
}

void LanguageManager::loadFromJson(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "Could not open" << filePath << ":" << file.errorString();
		return;
	}

	QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
	if (doc.isNull() || !doc.isObject())
	{
		qWarning() << "Invalid JSON format in" << filePath;
		return;
	}

	QJsonObject obj = doc.object();
	QJsonArray langArray = obj["languages"].toArray();
	for (const QJsonValue& value : langArray)
	{
		QJsonObject langObj = value.toObject();
		QString displayName = langObj["displayName"].toString();
		QString ocrCode = langObj["ocrCode"].toString();
		QString apiCode = langObj["apiCode"].toString();
		addLanguage(displayName, ocrCode, apiCode);
	}
}

void LanguageManager::addDefaultLanguages()
{
	addLanguage("English", "eng", "en");
	addLanguage("Russian", "rus", "ru");
	addLanguage("Ukrainian", "ukr", "uk");
	addLanguage("Spanish", "spa", "es");
	addLanguage("French", "fra", "fr");
}
