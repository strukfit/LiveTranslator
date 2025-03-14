#ifndef SCREENGRABBER_H
#define SCREENGRABBER_H

#include <QDialog>
#include <QWidget>
#include <QRubberBand>
#include <QMouseEvent>
#include <opencv2/core/mat.hpp>

class ScreenGrabber : public QDialog {
    Q_OBJECT

public:
    explicit ScreenGrabber(QScreen* screen, QWidget* parent = nullptr);
    ~ScreenGrabber();

    cv::Mat getCapturedImage() const { return capturedImage; }
    QRect getCapturectRect() const { return rubberBand ? rubberBand->geometry() : QRect(); }
    static QList<ScreenGrabber*> createForAllScreens(QWidget* parent);

signals:
    void captureCompleted();
    void captureCancelled();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void captureScreen();

    QRubberBand* rubberBand;
    QScreen* associatedScreen;
    QPoint origin;
    cv::Mat capturedImage;
    bool capturing;
};

#endif // SCREENGRABBER_H