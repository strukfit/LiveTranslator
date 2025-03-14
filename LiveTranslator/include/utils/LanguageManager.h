#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QString>
#include <QComboBox>

class LanguageManager {
public:
    LanguageManager(const QString& jsonFilePath = ":/resources/languages.json");

    void addLanguage(const QString& displayName, const QString& ocrCode, const QString& apiCode);
    QString getOcrCode(const QString& displayName) const;
    QString getApiCode(const QString& displayName) const;
    QStringList getLanguagesDisplayNames() const;

private:
    struct Language {
        QString ocrCode;
        QString apiCode;
    };
    QMap<QString, Language> languages; // displayName -> {ocrCode, apiCode}
    void loadFromJson(const QString& filePath);
    void addDefaultLanguages();
};

#endif // LANGUAGEMANAGER_H