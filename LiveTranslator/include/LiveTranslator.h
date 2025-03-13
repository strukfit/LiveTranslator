#pragma once

#include <QtWidgets/QMainWindow>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include "ui_LiveTranslator.h"

class LiveTranslator : public QMainWindow
{
    Q_OBJECT

public:
    LiveTranslator(QWidget *parent = nullptr);
    ~LiveTranslator();

private:
    Ui::LiveTranslatorClass ui;
};
