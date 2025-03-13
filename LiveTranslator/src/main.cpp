#include "LiveTranslator.h"
#include <QtWidgets/QApplication>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LiveTranslator w;
    w.show();
    return a.exec();
}
