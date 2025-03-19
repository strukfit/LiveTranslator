#include "translators/LibreTranslator.h"
#include "utils/RESTApiHandler.h"
#include "utils/NetworkErrorHandler.h"
#include <QDir>
#include <QUrl>
#include <QTimer>
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QProgressDialog>

LibreTranslator::LibreTranslator(const QString& serverPath, const QString& host, QObject* parent)
	: Translator(parent),
	m_serverPath(QDir::currentPath() + "/" + serverPath),
	m_venvPath(QDir::currentPath() + "/" + serverPath + "/venv"),
	m_modelsPath(QDir::currentPath() + "/" + serverPath + "/models"),
	m_host(host),
	m_libreProcess(new QProcess(this)),
	m_installProcess(new QProcess(this)),
	m_installDialog(nullptr),
	m_loadedLanguages(),
	m_serverReady(false)
{
	QDir serverDir(m_serverPath);
	if (!serverDir.exists()) 
	{
		serverDir.mkpath(".");
	}

	if (!checkServerExists() || !QDir(m_venvPath).exists())
	{
		createVirtualEnv();
		installServer();
	}

	loadExistingLanguageModels();
	ensureLanguageSupport("en", "ru");
	startServer(m_loadedLanguages);
}

LibreTranslator::~LibreTranslator()
{
	if (m_libreProcess && m_libreProcess->state() == QProcess::Running)
	{
		m_libreProcess->terminate();
		if (!m_libreProcess->waitForFinished(3000)) 
		{
			m_libreProcess->kill();
			qDebug() << "LibreTranslate process killed";
		}
		else 
		{
			qDebug() << "LibreTranslate process terminated gracefully";
		}
	}
}

void LibreTranslator::translate(const QString& text, const QString& sourceLang, const QString& targetLang)
{
	if (sourceLang == targetLang) return;

	ensureLanguageSupport(sourceLang, targetLang);

	if (!m_serverReady)
	{
		qDebug() << "Server not ready, waiting...";
		waitForServerReady();
	}

	QUrl url(m_host + "/translate");

	QJsonObject json;
	json["q"] = text;
	json["source"] = sourceLang == "auto" ? "auto" : sourceLang;
	json["target"] = targetLang;
	json["format"] = "text";

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

		if (NetworkErrorHandler::handleResponseErrors(responseObj, errorMessage)) {
			emit translationError(errorMessage);
			return;
		}

		QString translated = responseObj["translatedText"].toString();
		emit translationFinished(translated);
	});
}

void LibreTranslator::ensureLanguageSupport(const QString& sourceLang, const QString& targetLang)
{
	bool needsRestart = false;

	if (!m_loadedLanguages.contains(sourceLang) || !m_loadedLanguages.contains(targetLang))
	{
		qDebug() << "Installing package for" << sourceLang << "->" << targetLang;
		QProcess* modelProcess = new QProcess(this);
		QString pythonPath = m_venvPath + "/Scripts/python";
		QString scriptPath = m_serverPath + "/install_model.py";
		QStringList arguments;
		arguments << scriptPath << sourceLang << targetLang << m_modelsPath;

		QStringList env = QProcessEnvironment::systemEnvironment().toStringList();
		env << QString("ARGOS_TRANSLATE_DIR=%1").arg(m_modelsPath);
		env << QString("PYTHONPATH=%1").arg(m_serverPath);
		modelProcess->setEnvironment(env);
		modelProcess->setWorkingDirectory(m_serverPath);

		connect(modelProcess, &QProcess::readyReadStandardOutput, this, [=]() {
			qDebug().noquote() << modelProcess->readAllStandardOutput().trimmed();
		});

		connect(modelProcess, &QProcess::readyReadStandardError, this, [=]() {
			qDebug().noquote() << "Error:" << modelProcess->readAllStandardError().trimmed();
		});

		modelProcess->start(pythonPath, arguments);

		connect(modelProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
			if (exitStatus == QProcess::NormalExit && exitCode == 0) {
				qDebug() << "Package" << sourceLang << "->" << targetLang << "installed successfully";

				if (!m_loadedLanguages.contains(sourceLang)) {
					m_loadedLanguages << sourceLang;
				}

				if (!m_loadedLanguages.contains(targetLang)) {
					m_loadedLanguages << targetLang;
				}
			}
			else {
				qDebug() << "Failed to install package" << sourceLang << "->" << targetLang << ":" << modelProcess->readAllStandardError();
				emit translationError("Failed to install package for " + sourceLang + " -> " + targetLang);
			}
			modelProcess->deleteLater();
		});

		modelProcess->waitForFinished(-1);
		needsRestart = true;
	}

	if (needsRestart) {
		startServer(m_loadedLanguages);
	}
}

bool LibreTranslator::startServer(const QStringList& languages)
{
	if (m_libreProcess->state() == QProcess::Running) {
		m_libreProcess->terminate();
		m_libreProcess->waitForFinished(3000);
	}

	QString pythonPath = m_venvPath + "/Scripts/python";
	QString mainPyPath = m_venvPath + "/Lib/site-packages/libretranslate/main.py";
	QStringList arguments;
	arguments << mainPyPath << "--load-only" << languages.join(",");

	if (languages.isEmpty())
	{
		qDebug() << "No languages loaded";
	}

	m_libreProcess->setWorkingDirectory(m_serverPath);
	QStringList env = QProcessEnvironment::systemEnvironment().toStringList();
	env << QString("ARGOS_TRANSLATE_DIR=%1").arg(m_modelsPath);
	env << QString("PYTHONPATH=%1").arg(m_serverPath);
	env << QString("PYTHONIOENCODING=utf-8");
	m_libreProcess->setEnvironment(env);

	m_serverReady = false;
	m_libreProcess->start(pythonPath, arguments);
	if (!m_libreProcess->waitForStarted(5000)) {
		qDebug() << "Failed to start LibreTranslate:" << m_libreProcess->errorString();
		return false;
	}

	qDebug() << "LibreTranslate server started from:" << m_serverPath;

	connect(m_libreProcess, &QProcess::readyReadStandardOutput, this, [=]() {
		qDebug() << "LibreTranslate output:" << m_libreProcess->readAllStandardOutput();
	});

	connect(m_libreProcess, &QProcess::readyReadStandardError, this, [=]() {
		qDebug() << "LibreTranslate error:" << m_libreProcess->readAllStandardError();
	});

	qDebug() << "Libretranslate server started succesfully.";
	return true;
}

bool LibreTranslator::checkServerExists()
{
	QDir venvDir(m_venvPath + "/Scripts");
	return venvDir.exists("python.exe") && QFile::exists(m_venvPath + "/Lib/site-packages/libretranslate/__init__.py");
}

bool LibreTranslator::isServerRunning()
{
	return m_libreProcess && m_libreProcess->state() == QProcess::Running;
}

bool LibreTranslator::installServer()
{
	m_installDialog = new QProgressDialog("Installing LibreTranslate server...", "Cancel", 0, 0, qobject_cast<QWidget*>(parent()));
	m_installDialog->setWindowModality(Qt::WindowModal);
	m_installDialog->setAutoClose(true);
	m_installDialog->show();

	// Activate venv and install libretranslate
	QString pythonPath = m_venvPath + "/Scripts/python";
	QStringList arguments;
	arguments << "-m" << "pip" << "install" << "libretranslate";

	m_installProcess->setWorkingDirectory(m_serverPath);
	m_installProcess->start(pythonPath, arguments);

	bool success = false;
	connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [&success, this](int exitCode, QProcess::ExitStatus exitStatus) {
		if (exitStatus == QProcess::NormalExit && exitCode == 0) {
			qDebug() << "LibreTranslate installed successfully in venv";
			success = true;
			m_installDialog->accept();
		}
		else {
			qDebug() << "Installation failed:" << m_installProcess->readAllStandardError();
			emit translationError("Failed to install LibreTranslate");
			m_installDialog->reject();
		}
		});

	connect(m_installProcess, &QProcess::readyReadStandardOutput, this, [=]() {
		qDebug() << "Install output:" << m_installProcess->readAllStandardOutput();
	});

	connect(m_installProcess, &QProcess::readyReadStandardError, this, [=]() {
		qDebug() << "Install error:" << m_installProcess->readAllStandardError();
	});

	connect(m_installDialog, &QProgressDialog::canceled, this, [=]() {
		m_installProcess->terminate();
		if (!m_installProcess->waitForFinished(3000)) {
			m_installProcess->kill();
		}
		emit translationError("LibreTranslate installation canceled");
	});

	m_installProcess->waitForFinished(-1);
	return success;
}

bool LibreTranslator::createVirtualEnv()
{
	m_installDialog = new QProgressDialog("Creating virtual environment...", "Cancel", 0, 0, qobject_cast<QWidget*>(parent()));
	m_installDialog->setWindowModality(Qt::WindowModal);
	m_installDialog->setAutoClose(true);
	m_installDialog->show();

	QStringList arguments;
	arguments << "-m" << "venv" << m_venvPath;

	m_installProcess->setWorkingDirectory(m_serverPath);
	m_installProcess->start("python", arguments);

	bool success = false;
	connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [&success, this](int exitCode, QProcess::ExitStatus exitStatus) {
		if (exitStatus == QProcess::NormalExit && exitCode == 0) {
			qDebug() << "Virtual environment created at:" << m_venvPath;
			success = true;
			m_installDialog->accept();
		}
		else {
			qDebug() << "Failed to create virtual environment:" << m_installProcess->readAllStandardError();
			emit translationError("Failed to create virtual environment");
			m_installDialog->reject();
		}
	});

	connect(m_installDialog, &QProgressDialog::canceled, this, [=]() {
		m_installProcess->terminate();
		if (!m_installProcess->waitForFinished(3000)) {
			m_installProcess->kill();
		}
		emit translationError("Virtual environment creation canceled");
	});

	m_installProcess->waitForFinished(-1);
	return success;
}

void LibreTranslator::loadExistingLanguageModels()
{
	QDir dir(m_modelsPath);
	QStringList filters;
	filters << "*.argosmodel";
	QStringList modelFiles = dir.entryList(filters, QDir::Files);
	for (const QString& file : modelFiles) {
		QStringList parts = file.split('-')[1].split('_');
		if (parts.size() >= 2) {
			QString sourceLang = parts[0];
			QString targetLang = parts[1].split('.').first();
			if (!m_loadedLanguages.contains(sourceLang)) {
				m_loadedLanguages << sourceLang;
			}
			if (!m_loadedLanguages.contains(targetLang)) {
				m_loadedLanguages << targetLang;
			}
		}
	}
	qDebug() << "Loaded languages from existing models:" << m_loadedLanguages;
}

void LibreTranslator::waitForServerReady()
{
	QTimer timer(this);
	timer.setInterval(500);
	timer.setSingleShot(false);

	int attempts = 0;
	const int maxAttempts = 20;

	connect(&timer, &QTimer::timeout, this, [&timer, &attempts, maxAttempts, this]() mutable {
		QUrl url(QUrl(m_host + "/languages"));
		QNetworkReply* reply = RESTApiHandler::instance()->get(url);

		connect(reply, &QNetworkReply::finished, this, [reply, &timer, &attempts, maxAttempts, this]() {
			if (reply->error() == QNetworkReply::NoError) {
				qDebug() << "Server is ready at:" << m_host;
				m_serverReady = true;
				timer.stop();
				reply->deleteLater();
			}
			else {
				attempts++;
				qDebug() << "Waiting for server... Attempt" << attempts << "of" << maxAttempts;
				if (attempts >= maxAttempts) {
					qDebug() << "Server failed to start within timeout";
					timer.stop();
					emit translationError("Server failed to start within timeout");
					reply->deleteLater();
				}
			}
			reply->deleteLater();
		});
	});
}
