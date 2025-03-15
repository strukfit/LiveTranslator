#include "translators/TranslatorFactory.h"
#include "translators/GoogleTranslator.h"

Translator* TranslatorFactory::createTranslator(TranslationApi::Type type, QObject* parent, const QString& apiKey)
{
	switch (type)
	{
	case TranslationApi::Type::GoogleTranslate:
		return new GoogleTranslator(apiKey, parent);
	case TranslationApi::Type::DeepLTranslate:
		return nullptr;
	case TranslationApi::Type::LibreTranslate:
		return nullptr;
	default:
		return nullptr;
	}
}
