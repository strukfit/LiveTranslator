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

private:
    void ensureLanguageSupport(const QString& sourceLang, const QString& targetLang);
    bool startServer(const QStringList& languages);
    bool checkServerExists();
    bool isServerRunning();
    bool installServer();
    bool createVirtualEnv();
    void loadExistingLanguageModels();
    void waitForServerReady();

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