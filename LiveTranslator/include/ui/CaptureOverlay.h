#ifndef CAPTUREOVERLAY_H
#define CAPTUREOVERLAY_H

#include <QWidget>
#include <QScreen>
#include <QDialog>

class CaptureOverlay : public QDialog {
    Q_OBJECT

public:
    explicit CaptureOverlay(QScreen* screen, const QRect& rect, QWidget* parent = nullptr);
    void updateGeometry(const QRect& rect);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QScreen* captureScreen;
    QRect captureRect;
};

#endif // CAPTUREOVERLAY_H