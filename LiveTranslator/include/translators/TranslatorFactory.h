#ifndef TRANSLATORFACTORY_H
#define TRANSLATORFACTORY_H

#include "Translator.h"
#include "utils/TranslationApi.h"

class TranslatorFactory {
public:
    static Translator* createTranslator(TranslationApi::Type type, QObject* parent = nullptr, const QString& apiKey = "");
};

#endif // TRANSLATORFACTORY_H