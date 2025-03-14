#include "capture/ScreenGrabber.h"
#include "processing/ImageProcessor.h"
#include <QScreen>
#include <QWidget>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QApplication>

ScreenGrabber::ScreenGrabber(QScreen* screen, QWidget* parent)
    : QDialog(parent), 
    rubberBand(nullptr), 
    associatedScreen(screen),
    capturing(false)
{
    setWindowFlags(Qt::Window);
    setStyleSheet("background: black;");
    setWindowOpacity(0.7);
    setMouseTracking(true);
    setGeometry(screen->geometry());
}

ScreenGrabber::~ScreenGrabber() {
    delete rubberBand;
}

QList<ScreenGrabber*> ScreenGrabber::createForAllScreens(QWidget* parent)
{
    QList<ScreenGrabber*> grabbers;
    QList<QScreen*> screens = QGuiApplication::screens();

    for (QScreen* screen : screens)
    {
        ScreenGrabber* grabber = new ScreenGrabber(screen, parent);
        grabbers.append(grabber);
    }

    return grabbers;
}

cv::Mat ScreenGrabber::captureArea(QScreen* screen, const QRect& rect)
{
    if (!screen || rect.isEmpty()) {
        return cv::Mat();
    }

    // Capture the screen area
    QPixmap screenshot = screen->grabWindow(0,
        rect.x(),
        rect.y(),
        rect.width(),
        rect.height()
    );

    // Convert QPixmap to QImage
    QImage image = screenshot.toImage();

    // Convert QImage to cv::Mat
    return ImageProcessor::qimageToMat(image);
}

void ScreenGrabber::mousePressEvent(QMouseEvent* event) {
    origin = event->pos();
    if (!rubberBand) {
        rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    }
    rubberBand->setGeometry(QRect(origin, QSize()));
    rubberBand->show();
    capturing = true;
}

void ScreenGrabber::mouseMoveEvent(QMouseEvent* event) {
    if (capturing && rubberBand) {
        rubberBand->setGeometry(QRect(origin, event->pos()).normalized());
    }
}

void ScreenGrabber::mouseReleaseEvent(QMouseEvent* event) {
    if (capturing) {
        rubberBand->hide();
        hide();
        captureScreen();
        capturing = false;
        emit captureCompleted();
    }
}

void ScreenGrabber::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        capturing = false;
        if (rubberBand) {
            rubberBand->hide();
        }
        hide();
        emit captureCancelled();
    }
}

void ScreenGrabber::captureScreen() {
    if (!associatedScreen) return;

    // Get the coordinates of the selected area
    QRect area = rubberBand->geometry();

    capturedImage = captureArea(associatedScreen, area);
}