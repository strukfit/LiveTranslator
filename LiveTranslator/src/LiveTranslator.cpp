#include "LiveTranslator.h"
#include "capture/ScreenGrabber.h"
#include "processing/ImageProcessor.h"
#include "ui/TranslationLabel.h"
#include "utils/LanguageManager.h"
#include "ui/CaptureOverlay.h"
#include "workers/OcrWorker.h"
#include "translators/Translator.h"
#include "translators/TranslatorFactory.h"
#include "utils/Settings.h"
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QCloseEvent>

LiveTranslator::LiveTranslator(QWidget *parent)
    : QMainWindow(parent),
    m_updateTimer(new QTimer(this)),
    m_translationLabel(new TranslationLabel(nullptr)),
    m_captureRect(QRect()),
    m_captureScreen(nullptr),
    m_languageManager(new LanguageManager(":/resources/languages.json")),
    m_sourceModel(new QStringListModel(this)),
    m_sourceProxy(new QSortFilterProxyModel(this)),
    m_targetModel(new QStringListModel(this)),
    m_targetProxy(new QSortFilterProxyModel(this)),
    m_captureOverlay(nullptr),
    m_trayIcon(new QSystemTrayIcon(this)),
    m_trayMenu(new QMenu(this)),
    m_translator(nullptr),
    m_settings(new Settings(this))
{
    ui.setupUi(this);

    setupTrayMenu();

    QStringList languages = m_languageManager->getLanguagesDisplayNames();
    m_sourceModel->setStringList(languages);
    m_targetModel->setStringList(languages);

    setupLanguagesProxyModels();
    ui.sourceLanguageComboBox->setModel(m_sourceProxy);
    ui.targetLanguageComboBox->setModel(m_targetProxy);

    ui.sourceLanguageComboBox->setCurrentText(m_settings->getSourceLanguage());
    ui.targetLanguageComboBox->setCurrentText(m_settings->getTargetLanguage());

    connect(
        ui.sourceLanguageComboBox, &QComboBox::currentIndexChanged,
        this, [this](const QString& text) { m_settings->setSourceLanguage(text); }
    );

    connect(
        ui.targetLanguageComboBox, &QComboBox::currentIndexChanged,
        this, [this](const QString& text) { m_settings->setTargetLanguage(text); }
    );

    setupTranslatorComboBox();
    updateTranslator(ui.translatorComboBox->currentIndex());

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &LiveTranslator::trayIconActivated);

    connect(ui.sourceSearchEdit, &QLineEdit::textChanged, this, &LiveTranslator::filterSourceLanguages);
    connect(ui.targetSearchEdit, &QLineEdit::textChanged, this, &LiveTranslator::filterTargetLanguages);
    
    connect(m_updateTimer, &QTimer::timeout, this, &LiveTranslator::updateTranslation);
    connect(ui.captureButton, &QPushButton::clicked, this, &LiveTranslator::startCapture);
}

LiveTranslator::~LiveTranslator()
{
    if (m_updateTimer)
    {
        m_updateTimer->stop();
    }

    if (m_captureOverlay)
    {
        m_captureOverlay->close();
        m_captureOverlay->deleteLater();
        m_captureOverlay = nullptr;
    }

    if (m_translationLabel)
    {
        m_translationLabel->close();
        m_translationLabel->deleteLater();
        m_translationLabel = nullptr;
    }

    m_trayIcon->deleteLater();
    m_trayMenu->deleteLater();

    delete m_languageManager;
}

void LiveTranslator::closeEvent(QCloseEvent* event)
{
    if (m_trayIcon->isVisible())
    {
        qDebug() << "hide window";
        hide();
        event->ignore();
        m_trayIcon->showMessage(
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
        m_captureRect = grabber->getCaptureRect();
        m_captureScreen = grabber->getAssociatedScreen();

        if (!m_captureOverlay)
        {
            m_captureOverlay = new CaptureOverlay(m_captureScreen, m_captureRect, nullptr);
        }
        else 
        {
            m_captureOverlay->updateGeometry(m_captureRect);
            m_captureOverlay->show();
        }

        QRect screenRect = m_captureRect.translated(m_captureScreen->geometry().topLeft());
        m_translationLabel->setGeometry(screenRect);

        ImageProcessor::processImage(captured);
        
        // First translation
        updateTranslation();

        // Start the timer to refresh every 2 seconds
        m_updateTimer->start(2000);
    }
}

void LiveTranslator::updateTranslation()
{
    if (!m_captureScreen || m_captureRect.isEmpty()) return;

    m_translationLabel->hide();
    cv::Mat img = ScreenGrabber::captureArea(m_captureScreen, m_captureRect);
    ImageProcessor::processImage(img);
    m_translationLabel->show();

    if (m_captureOverlay)
    {
        m_captureOverlay->updateGeometry(m_captureRect);
    }

    QString sourceLangName = ui.sourceLanguageComboBox->currentText();
    QString ocrCode = m_languageManager->getOcrCode(sourceLangName);

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
    m_sourceProxy->setFilterWildcard(filter + '*'); // Filter with partial match support
    if (ui.sourceLanguageComboBox->currentIndex() == -1 && m_sourceProxy->rowCount() > 0)
    {
        ui.sourceLanguageComboBox->setCurrentIndex(0);
    }
}

void LiveTranslator::filterTargetLanguages(const QString& filter)
{
    m_targetProxy->setFilterWildcard(filter + "*"); // Filter with partial match support
    if (ui.targetLanguageComboBox->currentIndex() == -1 && m_targetProxy->rowCount() > 0)
    {
        ui.targetLanguageComboBox->setCurrentIndex(0);
    }
}

void LiveTranslator::translateText(const QString& text)
{
    m_translationLabel->setGeometry(m_captureRect.translated(m_captureScreen->geometry().topLeft()));
    m_translationLabel->setText(text);
    m_translationLabel->adjustSize();
    m_translationLabel->show();
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
    m_trayIcon->hide();
    QApplication::quit();
}

void LiveTranslator::updateTranslator(int index)
{
    if (m_translator)
    {
        m_translator->deleteLater();
        m_translator = nullptr;
    }

    TranslationApi::Type type = static_cast<TranslationApi::Type>(ui.translatorComboBox->itemData(index).toInt());
    m_translator = TranslatorFactory::createTranslator(type, this, m_settings->getApiKey(type));
}

void LiveTranslator::setupLanguagesProxyModels()
{
    m_sourceProxy->setSourceModel(m_sourceModel);
    m_sourceProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_sourceProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_targetProxy->setSourceModel(m_targetModel);
    m_targetProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_targetProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
}

void LiveTranslator::setupTrayMenu()
{
    m_trayIcon->setIcon(/*QIcon(":/icons/app_icon.png")*/QIcon::fromTheme("dialog-information"));
    m_trayIcon->setToolTip("Live Translator");
    m_trayIcon->show();

    QAction* showAction = new QAction("Show", this);
    QAction* quitAction = new QAction("Quit", this);
    m_trayMenu->addAction(showAction);
    m_trayMenu->addAction(quitAction);
    m_trayIcon->setContextMenu(m_trayMenu);

    connect(showAction, &QAction::triggered, this, &LiveTranslator::showWindow);
    connect(quitAction, &QAction::triggered, this, &LiveTranslator::quitApplication);
}

void LiveTranslator::setupTranslatorComboBox()
{
    for (const TranslationApi::Type type : TranslationApi::allValues())
    {
        ui.translatorComboBox->addItem(TranslationApi::toString(type), static_cast<int>(type));
    }
    ui.translatorComboBox->setCurrentIndex(static_cast<int>(m_settings->getTranslatorType()));

    connect(
        ui.translatorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &LiveTranslator::updateTranslator
    );
}
