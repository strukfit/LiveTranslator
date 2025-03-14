#include <QObject>
#include <QString>
#include <opencv2/core/mat.hpp>

class OcrWorker : public QObject {
    Q_OBJECT
public:
    OcrWorker(cv::Mat img, QString ocrCode) : img(std::move(img)), ocrCode(std::move(ocrCode)) {}

signals:
    void finished(QString text);

public slots:
    void process();

private:
    cv::Mat img;
    QString ocrCode;
};