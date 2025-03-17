#pragma once

#include <QtWidgets/QMainWindow>
#include <QSystemTrayIcon>
#include <QCache>
#include "ui_LiveTranslator.h"
#include "utils/HotkeyActions.h"

class QTimer;
class QScreen;
class TranslationLabel;
class LanguageManager;
class QStringListModel;
class QSortFilterProxyModel;
class CaptureOverlay;
class Translator;
class Settings;
class OcrWorker;
class HotkeyManager;
class QProgressDialog;

class LiveTranslator : public QMainWindow
{
    Q_OBJECT

public:
    LiveTranslator(QWidget *parent = nullptr);
    ~LiveTranslator() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void startCapture();
    void stopCapture();
    void processCapturedImage(class ScreenGrabber* grabber);
    void updateTranslation();
    void filterSourceLanguages(const QString& filter);
    void filterTargetLanguages(const QString& filter);
    void translateText(const QString& text, const QString& sourceLang, const QString& targetLang);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void showWindow();
    void quitApplication();
    void updateTranslator(int index);
    void updateTranslationLabel(const QString& text);
    void onHotkeyTriggered(HotkeyActions::Action action);
    void showDownloadProgress(const QString& langCode);

private:
    void setupLanguagesProxyModels();
    void setupTrayMenu();
    void setupTranslatorComboBox();
    void stopOcrWorkers();

    Ui::LiveTranslatorClass ui;
    QTimer* m_updateTimer;
    QRect m_captureRect;
    QScreen* m_captureScreen;
    QStringListModel* m_sourceModel;
    QSortFilterProxyModel* m_sourceProxy;
    QStringListModel* m_targetModel;
    QSortFilterProxyModel* m_targetProxy;
    QSystemTrayIcon* m_trayIcon;
    TranslationLabel* m_translationLabel;
    QMenu* m_trayMenu;
    LanguageManager* m_languageManager;
    CaptureOverlay* m_captureOverlay;
    Translator* m_translator;
    Settings* m_settings;
    HotkeyManager* m_hotkeyManager;
    QProgressDialog* m_downloadProgress;

    QList<QPair<QThread*, OcrWorker*>> m_ocrTasks;
    QCache<QString, QString> m_translationCache;
};
