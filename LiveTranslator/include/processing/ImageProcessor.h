#include <opencv2/core/mat.hpp>
#include <QImage>
#include <QString> 

class ImageProcessor
{
public:
	static cv::Mat qimageToMat(const QImage& image);
	static void processImage(cv::Mat& img);
	static QString recognizeText(const cv::Mat& img, const char* lang);
};