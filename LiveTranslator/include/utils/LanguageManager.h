#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QString>
#include <QComboBox>

class LanguageManager : public QObject
{
    Q_OBJECT
public:
    LanguageManager(const QString& jsonFilePath = ":/resources/languages.json", QObject* parent = nullptr);

    void addLanguage(const QString& displayName, const QString& ocrCode, const QString& apiCode);
    void ensureLanguageAvailible(const QString& ocrCode, std::function<void(bool)> callback = 0);
    QString getOcrCode(const QString& displayName) const;
    QString getApiCode(const QString& displayName) const;
    QString getDisplayNameByOcrCode(const QString& ocrCode) const;
    QString getTessdataPath() const;
    QStringList getLanguagesDisplayNames() const;

signals:
    void languageDownloadStarted(const QString& langCode);
    void languageDownloadFinished(const QString& langCode, bool success);

private:
    struct Language {
        QString ocrCode;
        QString apiCode;
    };
    void loadFromJson(const QString& filePath);
    void downloadLanguage(const QString& langCode);
    void addDefaultLanguages();

    QMap<QString, Language> languages; // displayName -> {ocrCode, apiCode}
    QSet<QString> m_activeDownloads;
    QString m_tessdataFullPath;
    QString m_tessdataPath;
};

#endif // LANGUAGEMANAGER_H