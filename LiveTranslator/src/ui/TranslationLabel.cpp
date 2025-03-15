#include "ui/TranslationLabel.h"
#include <QPainter>
#include <QMouseEvent>

TranslationLabel::TranslationLabel(QWidget *parent)
	: QLabel(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setAttribute(Qt::WA_ShowWithoutActivating);
	setFocusPolicy(Qt::NoFocus);
	hide();
}

TranslationLabel::~TranslationLabel()
{}

void TranslationLabel::mousePressEvent(QMouseEvent* event)
{
	event->ignore();
}

void TranslationLabel::mouseMoveEvent(QMouseEvent* event)
{
	event->ignore();
}

void TranslationLabel::mouseReleaseEvent(QMouseEvent* event)
{
	event->ignore();
}