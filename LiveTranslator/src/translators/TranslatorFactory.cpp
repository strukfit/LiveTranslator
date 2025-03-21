#include "translators/TranslatorFactory.h"
#include "translators/GoogleTranslator.h"
#include "translators/LibreTranslator.h"
#include "translators/DeepLTranslator.h"

Translator* TranslatorFactory::createTranslator(TranslationApi::Type type, QObject* parent, const QString& apiKey)
{
	switch (type)
	{
	case TranslationApi::Type::GoogleTranslate:
		return new GoogleTranslator(apiKey, parent);
	case TranslationApi::Type::DeepLTranslate:
		return new DeepLTranslator(apiKey, parent);
	case TranslationApi::Type::LibreTranslate:
		return new LibreTranslator("libretranslate_server", "http://localhost:5000", parent);;
	default:
		return nullptr;
	}
}
