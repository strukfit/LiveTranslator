#include "LiveTranslator.h"
#include "capture/ScreenGrabber.h"
#include "processing/ImageProcessor.h"
#include "ui/TranslationLabel.h"
#include "utils/LanguageManager.h"
#include "ui/CaptureOverlay.h"
#include "workers/OcrWorker.h"
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QCloseEvent>

LiveTranslator::LiveTranslator(QWidget *parent)
    : QMainWindow(parent),
    updateTimer(new QTimer(this)),
    translationLabel(new TranslationLabel(nullptr)),
    captureRect(QRect()),
    captureScreen(nullptr),
    languageManager(new LanguageManager(":/resources/languages.json")),
    sourceModel(new QStringListModel(this)),
    sourceProxy(new QSortFilterProxyModel(this)),
    targetModel(new QStringListModel(this)),
    targetProxy(new QSortFilterProxyModel(this)),
    captureOverlay(nullptr),
    trayIcon(new QSystemTrayIcon(this)),
    trayMenu(new QMenu(this))
{
    ui.setupUi(this);

    trayIcon->setIcon(/*QIcon(":/icons/app_icon.png")*/QIcon::fromTheme("dialog-information"));
    trayIcon->setToolTip("Live Translator");
    trayIcon->show();

    QAction* showAction = new QAction("Show", this);
    QAction* quitAction = new QAction("Quit", this);
    trayMenu->addAction(showAction);
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);

    QStringList languages = languageManager->getLanguagesDisplayNames();
    sourceModel->setStringList(languages);
    targetModel->setStringList(languages);

    setupLanguagesProxyModels();
    ui.sourceLanguageComboBox->setModel(sourceProxy);
    ui.targetLanguageComboBox->setModel(targetProxy);

    ui.targetLanguageComboBox->setCurrentText("English");

    connect(trayIcon, &QSystemTrayIcon::activated, this, &LiveTranslator::trayIconActivated);
    connect(showAction, &QAction::triggered, this, &LiveTranslator::showWindow);
    connect(quitAction, &QAction::triggered, this, &LiveTranslator::quitApplication);

    connect(ui.sourceSearchEdit, &QLineEdit::textChanged, this, &LiveTranslator::filterSourceLanguages);
    connect(ui.targetSearchEdit, &QLineEdit::textChanged, this, &LiveTranslator::filterTargetLanguages);
    
    connect(updateTimer, &QTimer::timeout, this, &LiveTranslator::updateTranslation);
    connect(ui.captureButton, &QPushButton::clicked, this, &LiveTranslator::startCapture);
}

LiveTranslator::~LiveTranslator()
{
    if (updateTimer) 
    {
        updateTimer->stop();
    }

    if (captureOverlay)
    {
        captureOverlay->close();
        captureOverlay->deleteLater();
        captureOverlay = nullptr;
    }

    if (translationLabel)
    {
        translationLabel->close();
        translationLabel->deleteLater();
        translationLabel = nullptr;
    }

    trayIcon->deleteLater();
    trayMenu->deleteLater();

    delete languageManager;
}

void LiveTranslator::closeEvent(QCloseEvent* event)
{
    if (trayIcon->isVisible())
    {
        qDebug() << "hide window";
        hide();
        event->ignore();
        trayIcon->showMessage(
            "Live Translator",
            "Application minimized to tray. Right-click the tray icon to restore or quit.",
            QSystemTrayIcon::Information, 
            3000
        );
    }
    else
    {
        event->accept();
        QApplication::quit();
    }
}

void LiveTranslator::startCapture()
{
    QList<ScreenGrabber*> grabbers = ScreenGrabber::createForAllScreens(this);

    auto deleteAllGrabbers = [=]() {
        for (ScreenGrabber* g : grabbers) {
            g->hide();
            g->deleteLater();
        }
    };

    for (ScreenGrabber* grabber : grabbers)
    {
        connect(grabber, &ScreenGrabber::captureCompleted, this, [=]() {
            processCapturedImage(grabber);
            deleteAllGrabbers();
        });

        connect(grabber, &ScreenGrabber::captureCancelled, this, [=]() {
            deleteAllGrabbers();
        });

        grabber->showFullScreen();
    }
}

void LiveTranslator::processCapturedImage(ScreenGrabber* grabber)
{
    cv::Mat captured = grabber->getCapturedImage();

    if (!captured.empty()) {
        captureRect = grabber->getCaptureRect();
        captureScreen = grabber->getAssociatedScreen();

        if (!captureOverlay) 
        {
            captureOverlay = new CaptureOverlay(captureScreen, captureRect, nullptr);
        }
        else 
        {
            captureOverlay->updateGeometry(captureRect);
            captureOverlay->show();
        }

        QRect screenRect = captureRect.translated(captureScreen->geometry().topLeft());
        translationLabel->setGeometry(screenRect);

        ImageProcessor::processImage(captured);
        
        // First translation
        updateTranslation();

        // Start the timer to refresh every 2 seconds
        updateTimer->start(2000);
    }
}

void LiveTranslator::updateTranslation()
{
    if (!captureScreen || captureRect.isEmpty()) return;

    translationLabel->hide();
    cv::Mat img = ScreenGrabber::captureArea(captureScreen, captureRect);
    ImageProcessor::processImage(img);
    translationLabel->show();

    if (captureOverlay)
    {
        captureOverlay->updateGeometry(captureRect);
    }

    QString sourceLangName = ui.sourceLanguageComboBox->currentText();
    QString ocrCode = languageManager->getOcrCode(sourceLangName);

    QThread* thread = new QThread();
    OcrWorker* worker = new OcrWorker(img, ocrCode);
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &OcrWorker::process);
    connect(worker, &OcrWorker::finished, this, [this, thread, worker](QString text) {
        qDebug() << "Recognized text: " + text;
        translateText(text);
        thread->quit();
        thread->deleteLater();
        worker->deleteLater();
    });
    thread->start();
}

void LiveTranslator::filterSourceLanguages(const QString& filter)
{
    sourceProxy->setFilterWildcard(filter + '*'); // Filter with partial match support
    if (ui.sourceLanguageComboBox->currentIndex() == -1 && sourceProxy->rowCount() > 0) 
    {
        ui.sourceLanguageComboBox->setCurrentIndex(0);
    }
}

void LiveTranslator::filterTargetLanguages(const QString& filter)
{
    targetProxy->setFilterWildcard(filter + "*"); // Filter with partial match support
    if (ui.targetLanguageComboBox->currentIndex() == -1 && targetProxy->rowCount() > 0) 
    {
        ui.targetLanguageComboBox->setCurrentIndex(0);
    }
}

void LiveTranslator::translateText(const QString& text)
{
    translationLabel->setGeometry(captureRect.translated(captureScreen->geometry().topLeft()));
    translationLabel->setText(text);
    translationLabel->adjustSize();
    translationLabel->show();
}

void LiveTranslator::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
    {
        showWindow();
    }
}

void LiveTranslator::showWindow()
{
    show();
    raise();
    activateWindow();
}

void LiveTranslator::quitApplication()
{
    trayIcon->hide();
    QApplication::quit();
}

void LiveTranslator::setupLanguagesProxyModels()
{
    sourceProxy->setSourceModel(sourceModel);
    sourceProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    sourceProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    targetProxy->setSourceModel(targetModel);
    targetProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    targetProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
}
