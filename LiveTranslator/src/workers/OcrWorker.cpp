#include "workers/OcrWorker.h"
#include "processing/ImageProcessor.h"

void OcrWorker::process() {
    if (!ocrCode.contains("eng", Qt::CaseInsensitive)) ocrCode = "eng+" + ocrCode;
    QString text = ImageProcessor::recognizeText(img, ocrCode.toStdString().c_str());
    emit finished(text);
}