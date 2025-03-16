#include <QObject>
#include <QString>
#include <opencv2/core/mat.hpp>

class OcrWorker : public QObject {
    Q_OBJECT
public:
    OcrWorker(cv::Mat img, QString ocrCode);

signals:
    void finished(QString text);

public slots:
    void process();
    void stop();

private:
    cv::Mat img;
    QString ocrCode;
    bool m_stopRequested;
};