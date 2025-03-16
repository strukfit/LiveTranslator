#include "ui/TranslationLabel.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTextBlockFormat>
#include <QTextCursor>

TranslationLabel::TranslationLabel(QWidget *parent)
	: QLabel(parent),
	m_padding(10)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setAttribute(Qt::WA_ShowWithoutActivating);
	setFocusPolicy(Qt::NoFocus);

	setWordWrap(true);
	//setAlignment(Qt::AlignTop | Qt::AlignLeft);
	setAlignment(Qt::AlignCenter);
	setTextFormat(Qt::RichText);
	
	QFont font = this->font();
	font.setPointSize(10);
	setFont(font);
	
	hide();
}

TranslationLabel::~TranslationLabel()
{}

void TranslationLabel::setText(const QString& text)
{
	QTextDocument doc;
	doc.setDefaultFont(font());
	doc.setPlainText(text);

	QTextBlockFormat blockFormat;
	blockFormat.setLineHeight(20, QTextLength::PercentageLength);
	blockFormat.setTopMargin(m_padding);
	blockFormat.setBottomMargin(m_padding);

	if (maximumWidth() > 0)
	{
		doc.setTextWidth(maximumWidth() - m_padding * 2);
	}

	QTextCursor cursor(&doc);
	cursor.select(QTextCursor::Document);
	cursor.mergeBlockFormat(blockFormat);

	QLabel::setText(doc.toHtml());
}

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