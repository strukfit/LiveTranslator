#include "processing/ImageProcessor.h"


cv::Mat ImageProcessor::qimageToMat(const QImage& image)
{
	 cv::Mat img = cv::Mat(image.height(), image.width(), CV_8UC4, 
		 const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();

	 // Convert from BGRA to BGR
	 cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);

	 return img;
}

void ImageProcessor::processImage(cv::Mat& img)
{
	cv::cvtColor(img, img, cv::COLOR_BGR2GRAY); // Gray color

	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe->setClipLimit(2.0);
	clahe->setTilesGridSize(cv::Size(8, 8));
	clahe->apply(img, img);

	//cv::threshold(img, img, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU); // Binarization
	cv::adaptiveThreshold(img, img, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 15, 5); // Binarization
}
