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
	if (!serverDir.exists()) serverDir.mkpath(".");
	QDir modelsDir(m_modelsPath);
	if (!modelsDir.exists()) modelsDir.mkpath(".");

	m_installDialog = new QProgressDialog("Installation", "Cancel", 0, 0, qobject_cast<QWidget*>(this));
	m_installDialog->setWindowModality(Qt::WindowModal);
	m_installDialog->setAutoClose(true);

	connect(this, &LibreTranslator::statusUpdated, m_installDialog, &QProgressDialog::setLabelText);
	connect(m_installDialog, &QProgressDialog::canceled, this, [=]() {
		m_installProcess->terminate();
		if (!m_installProcess->waitForFinished(3000)) {
			m_installProcess->kill();
		}
	});

	connect(m_libreProcess, &QProcess::readyReadStandardOutput, this, [=] {
		QString output = QString::fromUtf8(m_libreProcess->readAllStandardOutput());
		qDebug().noquote() << "LibreTranslate output:" << output;
		emit statusUpdated(output);
	});

	connect(m_libreProcess, &QProcess::readyReadStandardError, this, [=]() {
		QString error = QString::fromUtf8(m_libreProcess->readAllStandardError());
		qDebug().noquote() << "LibreTranslate error:" << error;
		emit statusUpdated("Error: " + error);
	});

	connect(m_installProcess, &QProcess::readyReadStandardOutput, this, [=]() {
		QString output = QString::fromUtf8(m_installProcess->readAllStandardOutput());
		qDebug().noquote() << "Install output:" << output;
		emit statusUpdated(output);
	});

	connect(m_installProcess, &QProcess::readyReadStandardError, this, [=]() {
		QString error = QString::fromUtf8(m_installProcess->readAllStandardError());
		qDebug().noquote() << "Install error:" << error;
		emit statusUpdated("Error: " + error);
	});

	connect(this, &LibreTranslator::initializationStepFinished, this, &LibreTranslator::onInitializationStepFinished);

	if (!QDir(m_venvPath).exists())
	{
		createVirtualEnv();
	}
	else
	{
		emit initializationStepFinished();
	}
}

LibreTranslator::~LibreTranslator()
{
	if (m_libreProcess && m_libreProcess->state() == QProcess::Running)
	{
		m_libreProcess->terminate();
		if (!m_libreProcess->waitForFinished(3000)) 
		{
			m_libreProcess->kill();
			emit statusUpdated("LibreTranslate process killed");
		}
		else 
		{
			emit statusUpdated("LibreTranslate process terminated gracefully");
		}
	}
	m_installDialog->deleteLater();
}

void LibreTranslator::translate(const QString& text, const QString& sourceLang, const QString& targetLang)
{
	if (sourceLang == targetLang) return;

	ensureLanguageSupport(sourceLang, targetLang, true);

	if (!m_serverReady)
	{
		emit statusUpdated("Server not ready, waiting...");
		QTimer::singleShot(0, this, &LibreTranslator::checkServerReady);
		connect(this, &LibreTranslator::serverReady, this, [=]() {
			translate(text, sourceLang, targetLang);
		}, Qt::UniqueConnection);
		return;
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

void LibreTranslator::onInitializationStepFinished()
{
	static int step = 0;
	switch (step)
	{
	case 0:
		step++;
		if (!checkServerExists())
		{
			installServer();
		}
		else
		{
			emit initializationStepFinished();
		}
		break;
	case 1:
		loadExistingLanguageModels();
		step++;
		emit initializationStepFinished();
		break;
	case 2:
		ensureLanguageSupport("en", "ru");
		step++;
		emit initializationStepFinished();
		break;
	case 3:
		startServer(m_loadedLanguages);
		step = 0;
		break;
	default: 
		break;
	}
}

void LibreTranslator::ensureLanguageSupport(const QString& sourceLang, const QString& targetLang, bool isRestartNeeded)
{
	if (!m_loadedLanguages.contains(sourceLang) || !m_loadedLanguages.contains(targetLang))
	{
		qDebug() << "Installing package for" << sourceLang << "->" << targetLang;
		QString pythonPath = m_venvPath + "/Scripts/python";
		QString scriptPath = m_serverPath + "/install_model.py";
		QStringList arguments;
		arguments << scriptPath << sourceLang << targetLang << m_modelsPath;

		QStringList env = QProcessEnvironment::systemEnvironment().toStringList();
		env << QString("ARGOS_TRANSLATE_DIR=%1").arg(m_modelsPath);
		env << QString("PYTHONPATH=%1").arg(m_serverPath);
		m_installProcess->setEnvironment(env);
		m_installProcess->setWorkingDirectory(m_serverPath);

		connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
			if (exitStatus == QProcess::NormalExit && exitCode == 0) {
				emit statusUpdated("Package " + sourceLang + " -> " + targetLang + " installed successfully");
				if (isRestartNeeded)
				{
					loadExistingLanguageModels();
					startServer(m_loadedLanguages);
				}
			}
			else {
				emit statusUpdated("Failed to install package " + sourceLang + " -> " + targetLang + ": " + QString::fromUtf8(m_installProcess->readAllStandardError()));
			}
		});

		m_installProcess->start(pythonPath, arguments);
	}
}

void LibreTranslator::startServer(const QStringList& languages)
{
	if (m_libreProcess->state() == QProcess::Running) {
		m_libreProcess->terminate();
		m_libreProcess->waitForFinished(3000);
	}

	QString pythonPath = m_venvPath + "/Scripts/python";
	QString mainPyPath = m_venvPath + "/Lib/site-packages/libretranslate/main.py";
	QStringList arguments;
	arguments << mainPyPath << "--load-only" << languages.join(",");

	m_libreProcess->setWorkingDirectory(m_serverPath);
	QStringList env = QProcessEnvironment::systemEnvironment().toStringList();
	env << QString("ARGOS_TRANSLATE_DIR=%1").arg(m_modelsPath);
	env << QString("PYTHONPATH=%1").arg(m_serverPath);
	env << QString("PYTHONIOENCODING=utf-8");
	m_libreProcess->setEnvironment(env);

	m_serverReady = false;
	m_libreProcess->start(pythonPath, arguments);

	connect(m_libreProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
		if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
			emit statusUpdated("Server stopped unexpectedly with exit code: " + QString::number(exitCode));
			m_serverReady = false;
		}
	});

	QTimer::singleShot(0, this, &LibreTranslator::checkServerReady);
}

bool LibreTranslator::checkServerExists()
{
	QDir venvDir(m_venvPath + "/Scripts");
	return venvDir.exists("python.exe") && QFile::exists(m_venvPath + "/Lib/site-packages/libretranslate/__init__.py");
}

void LibreTranslator::checkServerReady()
{
	static int attempts = 0;
	const int maxAttempts = 10;

	QUrl url(QUrl(m_host + "/languages"));
	QNetworkReply* reply = RESTApiHandler::instance()->get(url);

	connect(reply, &QNetworkReply::finished, this, [maxAttempts, reply, this]() {
		if (reply->error() == QNetworkReply::NoError) {
			m_serverReady = true;
			attempts = 0;
			emit statusUpdated("Server is ready at: " + m_host);
			emit serverReady();
		}
		else 
		{
			attempts++;
			if (attempts >= maxAttempts)
			{
				attempts = 0;
				m_serverReady = false;

				QString errorDetails = m_libreProcess->readAllStandardError();

				if (!errorDetails.isEmpty()) {
					emit statusUpdated("Server failed to start: " + errorDetails);
					return;
				}

				emit statusUpdated("Server failed to start after " + QString::number(maxAttempts) + " attempts.");
				return;
			}

			emit statusUpdated("Waiting for server... (Attempt " + QString::number(attempts) + " of " + QString::number(maxAttempts) + ")");
			QTimer::singleShot(500, this, &LibreTranslator::checkServerReady);
		}
		reply->deleteLater();
	});
}

bool LibreTranslator::isServerRunning()
{
	return m_libreProcess && m_libreProcess->state() == QProcess::Running;
}

void LibreTranslator::installServer()
{
	m_installDialog->show();
	emit statusUpdated("Installing LibreTranslate server...");

	// Activate venv and install libretranslate
	QString pythonPath = m_venvPath + "/Scripts/python";
	QStringList arguments;
	arguments << "-m" << "pip" << "install" << "libretranslate";

	m_installProcess->setWorkingDirectory(m_serverPath);

	connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
		if (exitStatus == QProcess::NormalExit && exitCode == 0) {
			emit statusUpdated("LibreTranslate installed successfully in venv");
			m_installDialog->accept();
			emit initializationStepFinished();
		}
		else {
			emit statusUpdated("Installation failed: " + QString::fromUtf8(m_installProcess->readAllStandardError()));
			m_installDialog->reject();
		}
	});

	m_installProcess->start(pythonPath, arguments);
}

void LibreTranslator::createVirtualEnv()
{
	m_installDialog->show();

	QStringList arguments;
	arguments << "-3.11" << "-m" << "venv" << m_venvPath;

	m_installProcess->setWorkingDirectory(m_serverPath);
	m_installProcess->setEnvironment({ QString("PYTHONIOENCODING=utf-8") });

	connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
		if (exitStatus == QProcess::NormalExit && exitCode == 0) {
			emit statusUpdated("Virtual environment created at: " + m_venvPath);
			m_installDialog->accept();
			emit initializationStepFinished();
		}
		else {
			emit statusUpdated("Failed to create virtual environment: " + QString::fromUtf8(m_installProcess->readAllStandardError()));
			m_installDialog->reject();
		}
	});

	emit statusUpdated("Creating virtual environment...");
	m_installProcess->start("py", arguments);
}

void LibreTranslator::loadExistingLanguageModels()
{
	QDir dir(m_modelsPath);
	QStringList filters;
	filters << "*.argosmodel";
	QStringList modelFiles = dir.entryList(filters, QDir::Files);
	m_loadedLanguages.clear();
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
