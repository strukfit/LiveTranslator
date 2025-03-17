#include "utils/LanguageManager.h"
#include "utils/RESTApiHandler.h"
#include "utils/NetworkErrorHandler.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>

LanguageManager::LanguageManager(const QString& jsonFilePath, QObject* parent)
	: QObject(parent),
	m_tessdataPath("resources/tessdata")
{
	m_tessdataFullPath = QDir::currentPath() + "/" + m_tessdataPath;

	QDir dir(m_tessdataFullPath);
	if (!dir.exists()) {
		dir.mkpath(".");
	}

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

void LanguageManager::ensureLanguageAvailible(const QString& ocrCode, std::function<void(bool)> callback)
{
	QFile file(m_tessdataFullPath + "/" + ocrCode + ".traineddata");
	if (file.exists())
	{
		qDebug() << "Language" << ocrCode << "already available.";
		callback(true);
		return;
	}

	if (m_activeDownloads.contains(ocrCode))
	{
		qDebug() << "Language" << ocrCode << "is already downloading, waiting for completion.";
		return;
	}

	qDebug() << "Language" << ocrCode << "not found, downloading...";
	m_activeDownloads.insert(ocrCode);
	emit languageDownloadStarted(ocrCode);
	downloadLanguage(ocrCode);

	connect(
		this, &LanguageManager::languageDownloadFinished, 
		this, [this, callback, ocrCode](const QString& downloadedCode, bool success) {
			if (downloadedCode == ocrCode) {
				m_activeDownloads.remove(ocrCode);
				callback(success);
			}
		}, Qt::SingleShotConnection
	);
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

QString LanguageManager::getDisplayNameByOcrCode(const QString& ocrCode) const
{
	for (auto it = languages.constBegin(); it != languages.constEnd(); ++it)
	{
		if (it.value().ocrCode == ocrCode)
		{
			return it.key();
		}
	}
	return QString();
}

QString LanguageManager::getTessdataPath() const
{
	return m_tessdataPath;
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

void LanguageManager::downloadLanguage(const QString& langCode)
{
	QUrl url(QString("https://raw.githubusercontent.com/tesseract-ocr/tessdata/main/%1.traineddata").arg(langCode));
	QNetworkReply* reply = RESTApiHandler::instance()->get(url);

	connect(reply, &QNetworkReply::finished, this, [=] {
		QString errorMessage;
		if (NetworkErrorHandler::handleReplyErrors(reply, errorMessage))
		{
			emit languageDownloadFinished(langCode, false);
			return;
		}

		QFile file(m_tessdataFullPath + "/" + langCode + ".traineddata");
		if (!file.open(QIODevice::WriteOnly))
		{
			qWarning() << "Failed to write" << langCode << "file.";
			emit languageDownloadFinished(langCode, false);
		}
		file.write(reply->readAll());
		file.flush();
		file.close();

		qDebug() << "Downloaded" << langCode << "to" << file.fileName();
		emit languageDownloadFinished(langCode, true);
	});
}

void LanguageManager::addDefaultLanguages()
{
	addLanguage("English", "eng", "en");
	addLanguage("Russian", "rus", "ru");
	addLanguage("Ukrainian", "ukr", "uk");
	addLanguage("Spanish", "spa", "es");
	addLanguage("French", "fra", "fr");
}
