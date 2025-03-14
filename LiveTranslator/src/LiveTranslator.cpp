#include "LiveTranslator.h"
#include "capture/ScreenGrabber.h"
#include "processing/ImageProcessor.h"
#include <QPushButton>
#include <QDebug>

LiveTranslator::LiveTranslator(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(ui.captureButton, &QPushButton::clicked, this, &LiveTranslator::startCapture);
}

LiveTranslator::~LiveTranslator()
{}

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
        // TODO : use captured image
        ImageProcessor::processImage(captured);
        cv::imshow("Captured", captured);
    }
}
