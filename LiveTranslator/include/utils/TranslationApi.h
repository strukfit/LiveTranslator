#ifndef TRANSLATIONAPI_H
#define TRANSLATIONAPI_H

#include <QString>
#include <QList>

#define ENUM_API_TYPES \
    ENUM_VALUE(GoogleTranslate, "Google Translate", "google") \
    ENUM_VALUE(DeepLTranslate, "DeepL Translate", "deepl") \
    ENUM_VALUE(LibreTranslate, "Libre Translate", "libretranslate")

#define ENUM_VALUE(name, string, service) name,

class TranslationApi {
public:
    enum class Type {
        ENUM_API_TYPES
    };

    static QString toString(Type type);
    static QString serviceName(Type type);
    static QList<Type> allValues();
};

#undef ENUM_VALUE
#endif // TRANSLATIONAPI_H