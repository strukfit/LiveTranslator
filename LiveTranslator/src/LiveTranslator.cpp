#include "LiveTranslator.h"
#include "ScreenGrabber.h"
#include <QPushButton>
#include <QDebug>

LiveTranslator::LiveTranslator(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(ui.captureButton, &QPushButton::clicked, this, [=]() {
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
                cv::Mat captured = grabber->getCapturedImage();

                if (!captured.empty()) {
                    cv::imshow("Captured", captured);
                }
                
                deleteAllGrabbers();
            });

            connect(grabber, &ScreenGrabber::captureCancelled, this, [=]() {
                deleteAllGrabbers();
            });

            grabber->showFullScreen();
        }
    });
}

LiveTranslator::~LiveTranslator()
{}
