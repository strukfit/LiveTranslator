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

private:
    void setupLanguagesProxyModels();

    Ui::LiveTranslatorClass ui;
    QTimer* updateTimer;
    TranslationLabel* translationLabel;
    QRect captureRect;
    QScreen* captureScreen;
    LanguageManager* languageManager;
    CaptureOverlay* captureOverlay;

    QStringListModel* sourceModel;
    QSortFilterProxyModel* sourceProxy;
    QStringListModel* targetModel;
    QSortFilterProxyModel* targetProxy;

    QSystemTrayIcon* trayIcon;
    QMenu* trayMenu;
};
