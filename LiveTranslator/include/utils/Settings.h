#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QSettings>
#include <utils/TranslationApi.h>

class Settings : public QObject {
    Q_OBJECT
public:
    explicit Settings(QObject* parent = nullptr);
    ~Settings() override;

    void saveValue(const QString& key, const QVariant& value);
    QVariant loadValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

    QString getApiKey(TranslationApi::Type type) const;
    QString getSourceLanguage() const;
    void setSourceLanguage(const QString& language);
    QString getTargetLanguage() const;
    void setTargetLanguage(const QString& language);
    TranslationApi::Type getTranslatorType() const;
    void setTranslatorType(TranslationApi::Type apiType);

    void saveSettings();
    void loadSettings();

private:
    QSettings m_settings;
    QString m_sourceLanguage;
    QString m_targetLanguage;
    TranslationApi::Type m_translatorType;
    QMap<TranslationApi::Type, QString> m_apiKeys;
};

#endif