#include "utils/Settings.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

Settings::Settings(QObject* parent)
	: QObject(parent),
	m_configPath(QDir::homePath() + "/.livetranslator/config.json"),
	m_sourceLanguage("eng"),
	m_targetLanguage("ru"),
	m_translatorType(TranslationApi::Type::GoogleTranslate)
{
	loadSettings();
}

Settings::~Settings()
{
	saveSettings();
}

QString Settings::getApiKey(TranslationApi::Type type) const
{
	QString service = TranslationApi::serviceName(type);
	QString envVar = service.toUpper() + "_API_KEY";
	QString apiKey = qgetenv(envVar.toUtf8());
	if (!apiKey.isEmpty()) {
		return apiKey;
	}

	QFile file(m_configPath);
	if (file.open(QIODevice::ReadOnly))
	{
		QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
		apiKey = doc.object()[service + "_api_key"].toString();
		if (!apiKey.isEmpty()) return apiKey;
	}

	qWarning() << "API Key for" << service << "not found!";
	return QString();
}

QString Settings::getSourceLanguage() const
{
	return m_sourceLanguage;
}

void Settings::setSourceLanguage(const QString& language)
{
	m_sourceLanguage = language;
}

QString Settings::getTargetLanguage() const
{
	return m_targetLanguage;
}

void Settings::setTargetLanguage(const QString& language)
{
	m_targetLanguage = language;
}

TranslationApi::Type Settings::getTranslatorType() const
{
	return m_translatorType;
}

void Settings::setTranslatorType(TranslationApi::Type type)
{
	m_translatorType = type;
}

void Settings::saveSettings() const
{
	QDir configDir(QDir::homePath() + "/.livetranslator/");
	if (!configDir.exists()) configDir.mkpath(".");

	QFile file(m_configPath);
	if (!file.open(QIODevice::WriteOnly))
	{
		qWarning() << "Failed to save settings to" << m_configPath;
	}

	QJsonObject json;
	json["source_language"] = m_sourceLanguage;
	json["target_language"] = m_targetLanguage;
	json["translator_type"] = static_cast<int>(m_translatorType);

	// Save existing API keys, if any
	QFile readFile(m_configPath);
	if (readFile.open(QIODevice::ReadOnly)) {
		QJsonDocument existingDoc = QJsonDocument::fromJson(readFile.readAll());
		QJsonObject existingJson = existingDoc.object();
		for (const QString& key : existingJson.keys()) {
			if (key.endsWith("_api_key")) {
				json[key] = existingJson[key];
			}
		}
	}

	file.write(QJsonDocument(json).toJson());
	file.close();
	qDebug() << "Settings saved to" << m_configPath;
}

void Settings::loadSettings()
{
	QFile file(m_configPath);
	if (file.open(QIODevice::ReadOnly))
	{
		QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
		QJsonObject json = doc.object();
		m_sourceLanguage = json["source_language"].toString("eng");
		m_targetLanguage = json["target_language"].toString("ru");
		m_translatorType = static_cast<TranslationApi::Type>(
			json["translator_type"].toInt(static_cast<int>(TranslationApi::Type::GoogleTranslate))
		);
		qDebug() << "Settings loaded from" << m_configPath;
	}
}
