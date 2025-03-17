#include <QObject>
#include <QString>
#include <opencv2/core/mat.hpp>

class OcrWorker : public QObject {
    Q_OBJECT
public:
    OcrWorker(QString tessdataPath, cv::Mat img, QString ocrCode);

signals:
    void finished(QString text);

public slots:
    void process();
    void stop();

private:
    cv::Mat m_img;
    QString m_ocrCode;
    bool m_stopRequested;
    QString m_tessdataPath;
};