#pragma once

#include <QLabel>
#include "ui_TranslationLabel.h"

class TranslationLabel : public QLabel
{
	Q_OBJECT

public:
	TranslationLabel(QWidget *parent = nullptr);
	~TranslationLabel();

private:
	Ui::TranslationLabelClass ui;
};
