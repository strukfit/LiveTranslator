#include "processing/ImageProcessor.h"
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <QDebug>

cv::Mat ImageProcessor::qimageToMat(const QImage& image)
{
	if (image.isNull()) return cv::Mat();

	cv::Mat img = cv::Mat(image.height(), image.width(), CV_8UC4, 
		 const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();

	 // Convert from BGRA to BGR
	cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);

	return img;
}

void ImageProcessor::processImage(cv::Mat& img)
{
	if (img.empty()) return;

	// Gray color
	cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);

	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe->setClipLimit(2.0);
	clahe->setTilesGridSize(cv::Size(8, 8));
	clahe->apply(img, img);

	// Binarization 
	cv::threshold(img, img, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	//cv::adaptiveThreshold(img, img, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 15, 5);

	// Invert image
	cv::bitwise_not(img, img);
}

QString ImageProcessor::recognizeText(const cv::Mat& img, const char* lang)
{
	if (img.empty()) return QString();

	tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
	const char* tessdataPath = "resources/tessdata";
	int initResult = ocr->Init(tessdataPath, lang);

	if (initResult != 0)
	{
		qWarning() << "Tesseract initialization failed with code:" << initResult << "for language:" << lang;
		delete ocr;
		return QString();
	}

	ocr->SetImage(img.data, img.cols, img.rows, 1, img.step);
	QString text = QString::fromUtf8(ocr->GetUTF8Text());
	ocr->End();
	delete ocr;

	return text;
}
