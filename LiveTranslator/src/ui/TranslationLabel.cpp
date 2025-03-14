#include "ui/TranslationLabel.h"

TranslationLabel::TranslationLabel(QWidget *parent)
	: QLabel(parent)
{
	ui.setupUi(this);
	hide();
}

TranslationLabel::~TranslationLabel()
{}
