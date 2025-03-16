#pragma once

#include <QLabel>
#include "ui_TranslationLabel.h"

class QMouseEvent;

class TranslationLabel : public QLabel
{
	Q_OBJECT

public:
	TranslationLabel(QWidget *parent = nullptr);
	~TranslationLabel();

	void setText(const QString& text);

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	Ui::TranslationLabelClass ui;
	int m_padding;
};
