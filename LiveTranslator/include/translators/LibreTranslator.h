#ifndef LIBRETRANSLATOR_H
#define LIBRETRANSLATOR_H

#include "translators/Translator.h"
#include <QProcess>
#include <QProgressDialog>
#include <QString>

class RESTApiHandler;

class LibreTranslator : public Translator
{
    Q_OBJECT
public:
    LibreTranslator(const QString& serverPath = "libretranslate_server",
        const QString& host = "http://localhost:5000",
        QObject* parent = nullptr);
    ~LibreTranslator() override;

    void translate(const QString& text, const QString& sourceLang, const QString& targetLang) override;

signals:
    void serverReady();
    void statusUpdated(const QString& status);
    void progressUpdated(int value, int maximum);
    void initializationStepFinished();

private slots:
    void checkServerReady();
    void onInitializationStepFinished();

private:
    void ensureLanguageSupport(const QString& sourceLang, const QString& targetLang, bool isRestartNeeded = false);
    void startServer(const QStringList& languages);
    bool checkServerExists();
    bool isServerRunning();
    void installServer();
    void createVirtualEnv();
    void loadExistingLanguageModels();

    QString m_serverPath;
    QString m_venvPath;
    QString m_modelsPath;
    QString m_host;
    QProcess* m_libreProcess;
    QProcess* m_installProcess;
    QProgressDialog* m_installDialog;
    QStringList m_loadedLanguages;
    bool m_serverReady;
};

#endif // LIBRETRANSLATOR_H