#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QString>
#include <utils/TranslationApi.h>

class Settings : public QObject {
    Q_OBJECT
public:
    explicit Settings(QObject* parent = nullptr);
    ~Settings() override;

    QString getApiKey(TranslationApi::Type type) const;

    QString getSourceLanguage() const;
    void setSourceLanguage(const QString& language);
    QString getTargetLanguage() const;
    void setTargetLanguage(const QString& language);
    TranslationApi::Type getTranslatorType() const;
    void setTranslatorType(TranslationApi::Type type);

    void saveSettings() const;
    void loadSettings();

private:
    QString m_configPath;
    QString m_sourceLanguage;
    QString m_targetLanguage;
    TranslationApi::Type m_translatorType;
};

#endif