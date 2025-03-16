#include "workers/OcrWorker.h"
#include "processing/ImageProcessor.h"


OcrWorker::OcrWorker(cv::Mat img, QString ocrCode)
    : img(std::move(img)), 
    ocrCode(std::move(ocrCode)),
    m_stopRequested(false)
{
}

void OcrWorker::stop()
{
    m_stopRequested = true;
}

void OcrWorker::process() {
    if (m_stopRequested)
    {
        emit finished("");
        return;
    }

    if (!ocrCode.contains("eng", Qt::CaseInsensitive)) ocrCode = "eng+" + ocrCode;
    QString text = ImageProcessor::recognizeText(img, ocrCode.toStdString().c_str());

    if (m_stopRequested)
    {
        emit finished("");
        return;
    }

    emit finished(text);
}

