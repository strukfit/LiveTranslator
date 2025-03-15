#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QString>
#include <QObject>

class Translator : public QObject {
    Q_OBJECT
public:
    explicit Translator(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Translator() = default;
    virtual void translate(const QString& text, const QString& sourceLang, const QString& targetLang) = 0;

signals:
    void translationFinished(const QString& translatedText);
    void translationError(const QString& error);
};

#endif // TRANSLATOR_H