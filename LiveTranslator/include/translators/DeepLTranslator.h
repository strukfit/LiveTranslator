#ifndef DEEPLTRANSLATOR_H
#define DEEPLTRANSLATOR_H

#include "Translator.h"
#include <QObject>

class QNetworkReply;

class DeepLTranslator : public Translator
{
    Q_OBJECT
public:
    explicit DeepLTranslator(const QString& apiKey, bool isApiFree = true, QObject* parent = nullptr);
    void translate(const QString& text, const QString& sourceLang, const QString& targetLang) override;

private:
    QString m_apiKey;
    QString m_apiEndpoint;
};

#endif // DEEPLTRANSLATOR_H