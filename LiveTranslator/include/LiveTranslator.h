#pragma once

#include <QtWidgets/QMainWindow>
#include <QSystemTrayIcon>
#include <windows.h>
#include "ui_LiveTranslator.h"

class QTimer;
class QScreen;
class TranslationLabel;
class LanguageManager;
class QStringListModel;
class QSortFilterProxyModel;
class CaptureOverlay;
class Translator;

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
    void processCapturedImage(class ScreenGrabber* grabber);
    void updateTranslation();
    void filterSourceLanguages(const QString& filter);
    void filterTargetLanguages(const QString& filter);
    void translateText(const QString& text);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void showWindow();
    void quitApplication();
    void updateTranslator(int index);

private:
    void setupLanguagesProxyModels();
    void setupTrayMenu();
    void setupTranslatorComboBox();

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
};
