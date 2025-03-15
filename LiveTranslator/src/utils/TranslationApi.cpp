#include "utils/TranslationApi.h"

QString TranslationApi::toString() const {
    switch (m_type) {
    case Type::GoogleTranslate: return "Google Translate";
    case Type::DeepLTranslate: return "DeepL Translate";
    case Type::LibreTranslate: return "Libre Translate";
    }
    return "Unknown API";
}

QList<TranslationApi> TranslationApi::allValues() {
    return {
        TranslationApi(Type::GoogleTranslate),
        TranslationApi(Type::DeepLTranslate),
        TranslationApi(Type::LibreTranslate)
    };
}