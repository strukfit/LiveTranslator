#include "ui/CaptureOverlay.h"
#include <QPainter>

CaptureOverlay::CaptureOverlay(QScreen* screen, const QRect& rect, QWidget* parent)
	: QDialog(parent),
	captureScreen(screen),
	captureRect(rect)
{
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setGeometry(captureScreen->geometry());
	show();
}

void CaptureOverlay::updateGeometry(const QRect& rect)
{
	captureRect = rect;
	update();
}

void CaptureOverlay::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QPen pen(Qt::red, 2, Qt::DashLine);
	painter.setPen(pen);
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(captureRect);
}
