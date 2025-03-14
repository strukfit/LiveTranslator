#include "workers/OcrWorker.h"
#include "processing/ImageProcessor.h"

void OcrWorker::process() {
    QString text = ImageProcessor::recognizeText(img, ocrCode.toStdString().c_str());
    emit finished(text);
}