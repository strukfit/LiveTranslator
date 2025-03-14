#include "LiveTranslator.h"
#include "capture/ScreenGrabber.h"
#include "processing/ImageProcessor.h"
#include "ui/TranslationLabel.h"
#include "utils/LanguageManager.h"
#include "ui/CaptureOverlay.h"
#include <QPushButton>
#include <QTimer>
#include <QDebug>
#include <QStringListModel>
#include <QSortFilterProxyModel>

LiveTranslator::LiveTranslator(QWidget *parent)
    : QMainWindow(parent),
    updateTimer(new QTimer(this)),
    translationLabel(new TranslationLabel(this)),
    captureRect(QRect()),
    captureScreen(nullptr),
    languageManager(new LanguageManager(":/resources/languages.json")),
    sourceModel(new QStringListModel(this)),
    sourceProxy(new QSortFilterProxyModel(this)),
    targetModel(new QStringListModel(this)),
    targetProxy(new QSortFilterProxyModel(this)),
    captureOverlay(nullptr)
{
    ui.setupUi(this);

    QStringList languages = languageManager->getLanguagesDisplayNames();
    sourceModel->setStringList(languages);
    targetModel->setStringList(languages);

    setupLanguagesProxyModels();
    ui.sourceLanguageComboBox->setModel(sourceProxy);
    ui.targetLanguageComboBox->setModel(targetProxy);

    ui.targetLanguageComboBox->setCurrentText("English");

    connect(ui.sourceSearchEdit, &QLineEdit::textChanged, this, &LiveTranslator::filterSourceLanguages);
    connect(ui.targetSearchEdit, &QLineEdit::textChanged, this, &LiveTranslator::filterTargetLanguages);

    connect(updateTimer, &QTimer::timeout, this, &LiveTranslator::updateTranslation);
    connect(ui.captureButton, &QPushButton::clicked, this, &LiveTranslator::startCapture);
}

LiveTranslator::~LiveTranslator()
{
    delete languageManager;
    delete captureOverlay;
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

        if (!captureOverlay) {
            captureOverlay = new CaptureOverlay(captureScreen, captureRect, this);
        }
        else {
            captureOverlay->updateGeometry(captureRect);
            captureOverlay->show();
        }

        ImageProcessor::processImage(captured);
        
        // First translation
        updateTranslation();

        translationLabel->setGeometry(captureRect);
        translationLabel->show();

        // Start the timer to refresh every 2 seconds
        updateTimer->start(2000);
    }
}

void LiveTranslator::updateTranslation()
{
    if (!captureScreen || captureRect.isEmpty()) return;

    cv::Mat img = ScreenGrabber::captureArea(captureScreen, captureRect);

    QString sourceLangName = ui.sourceLanguageComboBox->currentText();
    QString ocrCode = languageManager->getOcrCode(sourceLangName);

    QString text = ImageProcessor::recognizeText(img, ocrCode.toStdString().c_str());

    qDebug() << "Recognized text: " + text; 

    if (captureOverlay)
    {
        captureOverlay->updateGeometry(captureRect);
    }
}

void LiveTranslator::filterSourceLanguages(const QString& filter)
{
    sourceProxy->setFilterWildcard(filter + '*'); // Filter with partial match support
    if (ui.sourceLanguageComboBox->currentIndex() == -1 && sourceProxy->rowCount() > 0) {
        ui.sourceLanguageComboBox->setCurrentIndex(0);
    }
}

void LiveTranslator::filterTargetLanguages(const QString& filter)
{
    targetProxy->setFilterWildcard(filter + "*"); // Filter with partial match support
    if (ui.targetLanguageComboBox->currentIndex() == -1 && targetProxy->rowCount() > 0) {
        ui.targetLanguageComboBox->setCurrentIndex(0);
    }
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
