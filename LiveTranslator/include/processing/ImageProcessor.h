#include <opencv2/opencv.hpp>
#include <QImage>

class ImageProcessor
{
public:
	static cv::Mat qimageToMat(const QImage& image);
	static void processImage(cv::Mat& img);
};