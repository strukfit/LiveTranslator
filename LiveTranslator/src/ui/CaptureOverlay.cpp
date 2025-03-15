#include "ui/CaptureOverlay.h"
#include <QPainter>

CaptureOverlay::CaptureOverlay(QScreen* screen, const QRect& rect, QWidget* parent)
	: QWidget(parent),
	captureScreen(screen),
	captureRect(rect)
{
	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setAttribute(Qt::WA_ShowWithoutActivating);
	setFocusPolicy(Qt::NoFocus);
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

	// Draw a semi-transparent background
	painter.setBrush(QColor(0, 0, 0, 100));
	painter.setPen(Qt::NoPen);
	painter.drawRect(captureRect);

	QPen pen(Qt::green, 2, Qt::DashLine);
	painter.setPen(pen);
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(captureRect);
}
