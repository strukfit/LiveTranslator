#include "utils/TranslationApi.h"

QString TranslationApi::toString(Type type) {
#define ENUM_VALUE(name, string, service) case Type::name: return string;
    switch (type)
    {
        ENUM_API_TYPES
    default: return "Unknown API";
    }
#undef ENUM_VALUE
}

QString TranslationApi::serviceName(Type type)
{
#define ENUM_VALUE(name, string, service) case Type::name: return service;
    switch (type)
    {
        ENUM_API_TYPES
    default: return "Unknown API";
    }
#undef ENUM_VALUE
}

QList<TranslationApi::Type> TranslationApi::allValues() 
{
#define ENUM_VALUE(name, string, service) Type::name,
    return { ENUM_API_TYPES };
#undef ENUM_VALUE
}