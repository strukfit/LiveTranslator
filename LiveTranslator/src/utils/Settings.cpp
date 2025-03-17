#include "utils/Settings.h"
#include <QDir>

Settings::Settings(QObject* parent)
	: QObject(parent),
	m_settings(QDir::homePath() + "/.livetranslator/settings.ini", QSettings::IniFormat),
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

void Settings::saveValue(const QString& key, const QVariant& value)
{
	m_settings.setValue(key, value);
}

QVariant Settings::loadValue(const QString& key, const QVariant& defaultValue) const
{
	return m_settings.value(key, defaultValue);
}

QString Settings::getApiKey(TranslationApi::Type apiType) const
{
	QString service = TranslationApi::serviceName(apiType);
	QString envVar = service.toUpper() + "_API_KEY";
	QString apiKey = qgetenv(envVar.toUtf8());
	
	if (!apiKey.isEmpty()) return apiKey;
	if (m_apiKeys.contains(apiType)) return m_apiKeys[apiType];

	apiKey = m_settings.value(service + "_api_key", "").toString();
	if (!apiKey.isEmpty()) {
		m_apiKeys[apiType] = apiKey;
		return apiKey;
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
	m_settings.setValue("source_language", m_sourceLanguage);
}

QString Settings::getTargetLanguage() const
{
	return m_targetLanguage;
}

void Settings::setTargetLanguage(const QString& language)
{
	m_targetLanguage = language;
	m_settings.setValue("target_language", m_targetLanguage);
}

TranslationApi::Type Settings::getTranslatorType() const
{
	return m_translatorType;
}

void Settings::setTranslatorType(TranslationApi::Type type)
{
	m_translatorType = type;
	m_settings.setValue("translator_type", static_cast<int>(m_translatorType));
}

void Settings::saveSettings()
{
	m_settings.setValue("source_language", m_sourceLanguage);
	m_settings.setValue("target_language", m_targetLanguage);
	m_settings.setValue("translator_type", static_cast<int>(m_translatorType));
	for (auto it = m_apiKeys.constBegin(); it != m_apiKeys.constEnd(); ++it)
	{
		QString service = TranslationApi::serviceName(it.key());
		m_settings.setValue(service + "_api_key", it.value());
	}
}

void Settings::loadSettings()
{
	QDir dir(QDir::homePath() + "/.livetranslator");
	if (!dir.exists()) {
		dir.mkpath(".");
	}

	m_sourceLanguage = m_settings.value("source_language", "eng").toString();
	m_targetLanguage = m_settings.value("target_language", "ru").toString();
	m_translatorType = static_cast<TranslationApi::Type>(
		m_settings.value("translator_type", static_cast<int>(TranslationApi::Type::GoogleTranslate)).toInt()
	);

	for (const TranslationApi::Type type : TranslationApi::allValues()) 
	{
		QString service = TranslationApi::serviceName(type);
		QString key = service + "_api_key";
		QString value = m_settings.value(key, "").toString();

		if (!value.isEmpty()) {
			m_apiKeys[type] = value;
		}
	}
}
