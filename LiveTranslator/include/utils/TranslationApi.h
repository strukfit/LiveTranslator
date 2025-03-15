#ifndef TRANSLATIONAPI_H
#define TRANSLATIONAPI_H

#include <QString>
#include <QList>

class TranslationApi {
public:
    enum class Type {
        GoogleTranslate,
        DeepLTranslate,
        LibreTranslate
    };

    explicit TranslationApi(Type type) : m_type(type) {}
    QString toString() const;
    Type type() const { return m_type; }
    static QList<TranslationApi> allValues();

private:
    Type m_type;
};

#endif // TRANSLATIONAPI_H