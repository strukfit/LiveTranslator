#ifndef GOOGLETRANSLATOR_H
#define GOOGLETRANSLATOR_H

#include "Translator.h"
#include <QString>

class RESTApiHandler;

class GoogleTranslator : public Translator {
    Q_OBJECT
public:
    explicit GoogleTranslator(const QString& apiKey, QObject* parent = nullptr);
    void translate(const QString& text, const QString& sourceLang, const QString& targetLang) override;

private:
    QString m_apiKey;
    const QString m_apiEndpoint = "https://translation.googleapis.com/language/translate/v2";
};

#endif // GOOGLETRANSLATOR_H