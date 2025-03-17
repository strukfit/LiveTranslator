#include "workers/OcrWorker.h"
#include "processing/ImageProcessor.h"
#include <QDebug>


OcrWorker::OcrWorker(QString tessdataPath, cv::Mat img, QString ocrCode)
    : m_img(std::move(img)), 
    m_ocrCode(std::move(ocrCode)),
    m_tessdataPath(std::move(tessdataPath)),
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

    if (!m_ocrCode.contains("eng", Qt::CaseInsensitive)) m_ocrCode = "eng+" + m_ocrCode;
    QString text = ImageProcessor::recognizeText(
        m_tessdataPath.toStdString().c_str(), 
        m_img, 
        m_ocrCode.toStdString().c_str()
    );

    if (m_stopRequested)
    {
        emit finished("");
        return;
    }

    emit finished(text);
}

