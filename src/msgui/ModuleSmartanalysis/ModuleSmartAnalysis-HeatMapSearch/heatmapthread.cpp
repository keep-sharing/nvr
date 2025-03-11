#include "heatmapthread.h"
#include "centralmessage.h"
#include <QElapsedTimer>
#include <QPainter>
#include <QtDebug>

HeatMapThread::HeatMapThread(QObject *parent)
    : QObject(parent)
{
    moveToThread(&m_thread);
    connect(&m_thread, SIGNAL(started()), this, SLOT(onThreadStarted()));
    m_thread.setObjectName("Qt-HeatMapThread");
    m_thread.start();
}

HeatMapThread::~HeatMapThread()
{
}

void HeatMapThread::stop()
{
    m_thread.quit();
    m_thread.wait();
}

void HeatMapThread::setBackgroundImage(const QImage &image)
{
    m_mutex.lock();
    qDebug() << "HeatMapThread::setBackgroundImage, size:" << image.size();
    m_backgroundImage = image;
    m_mutex.unlock();
}

void HeatMapThread::makeHeatMap(const DrawHeatMapData &drawHeatMapData)
{
    qDebug() << "HeatMapThread::makeHeatMap, corridorMode:" << drawHeatMapData.corridorMode << ", imageRotation:" << drawHeatMapData.imageRotation << ", mode:" << drawHeatMapData.mode;
    QMetaObject::invokeMethod(this, "onMakeHeatMap", Q_ARG(QString, drawHeatMapData.text), Q_ARG(int, drawHeatMapData.corridorMode), Q_ARG(int, drawHeatMapData.imageRotation), Q_ARG(int, drawHeatMapData.mode));
}

void HeatMapThread::saveSpaceHeatMap(const QString &filePath)
{
    QMetaObject::invokeMethod(this, "onSaveSpaceHeatMap", Q_ARG(QString, filePath));
}

void HeatMapThread::initializeColorImage()
{
    m_colorImage = QImage(255, 30, QImage::Format_ARGB32);

    QPainter painter(&m_colorImage);
    painter.setPen(Qt::NoPen);
    QLinearGradient linear(0, 0, 255, 0);
    linear.setColorAt(0.25, Qt::blue);
    linear.setColorAt(0.55, Qt::green);
    linear.setColorAt(0.85, Qt::yellow);
    linear.setColorAt(1, Qt::red);
    painter.setBrush(linear);
    painter.drawRect(m_colorImage.rect());
}

void HeatMapThread::onThreadStarted()
{
    initializeColorImage();
}

void HeatMapThread::onMakeHeatMap(const QString &text, int corridorMode, int imageRotation, int mode)
{
    QElapsedTimer timer;
    timer.start();

    QImage alphaImage;
    QImage heatMapImage;

    qDebug() << QString("HeatMapThread::onMakeHeatMap, initialize: %1ms").arg(timer.elapsed());

    int max = 0;
    int min = 0;
    int width = 192;
    int height = 192;
    //QRegExp rx(R"("max":(\d+),"min":(\d+),"map_w":(\d+),"map_h":(\d+),"data":\[(.*)\])");
    QRegExp rx(R"tt((.*),"data":\[(.*)\])tt");
    if (rx.indexIn(text) != -1) {
        QString strParameters = rx.cap(1);

        QRegExp rxMaxMin(R"("max":(\d+),"min":(\d+))");
        if (rxMaxMin.indexIn(strParameters) != -1) {
            QString strMax = rxMaxMin.cap(1);
            QString strMin = rxMaxMin.cap(2);
            max = strMax.toInt();
            min = strMin.toInt();
        }
        QRegExp rxWidthHeight(R"("map_w":(\d+),"map_h":(\d+))");
        if (rxWidthHeight.indexIn(strParameters) != -1) {
            QString strWidth = rxWidthHeight.cap(1);
            QString strHeight = rxWidthHeight.cap(2);
            width = strWidth.toInt();
            height = strHeight.toInt();
        }

        alphaImage = QImage(width, height, QImage::Format_ARGB32);
        alphaImage.fill(Qt::transparent);
        heatMapImage = QImage(width, height, QImage::Format_ARGB32);
        heatMapImage.fill(Qt::transparent);

        QString strData = rx.cap(2);
        QStringList dataList = strData.split("},{");
        qDebug() << QString("HeatMapThread::onMakeHeatMap, max: %1, min: %2, width: %3, height: %4, size: %5").arg(max).arg(min).arg(width).arg(height).arg(dataList.size());
        QRegExp rx2(R"("x":(\d+),"y":(\d+),"value":(\d+))");

        qreal radius = 1.9;

        QPainter alphaPainter(&alphaImage);
        alphaPainter.setRenderHint(QPainter::Antialiasing);
        for (int i = 0; i < dataList.size(); ++i) {
            QString temp = dataList.at(i);
            if (rx2.indexIn(temp) != -1) {
                int x = rx2.cap(1).toInt();
                int y = rx2.cap(2).toInt();
                int value = rx2.cap(3).toInt();

                int alpha = (qreal)(value - min) / (max - min) * 255;

                QRadialGradient gradient(x, y, radius);
                gradient.setColorAt(0, QColor(0, 0, 0, alpha));
                gradient.setColorAt(1, QColor(0, 0, 0, 0));

                alphaPainter.setPen(Qt::NoPen);
                alphaPainter.setBrush(gradient);
                alphaPainter.drawEllipse(QPointF(x, y), radius, radius);
            }
        }

        qDebug() << QString("HeatMapThread::onMakeHeatMap, drawEllipse: %1ms").arg(timer.elapsed());

        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                int alpha = qAlpha(alphaImage.pixel(x, y));
                if (alpha == 0) {
                    continue;
                }
                int finalAlpha = alpha;
                if (finalAlpha > 128) {
                    finalAlpha = 128;
                }
                QColor color = m_colorImage.pixel(alpha - 1, 15);
                color.setAlpha(finalAlpha);
                heatMapImage.setPixel(x, y, color.rgba());
            }
        }

        qDebug() << QString("HeatMapThread::onMakeHeatMap, setPixel: %1ms").arg(timer.elapsed());
    } else {
        qWarning() << "error heatmap data:" << text;
    }
    //
    m_mutex.lock();
    QPainter painter(&m_backgroundImage);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QRect heatRect;
    switch (mode) {
    case ModeFisheye:
        heatRect.setWidth(qMin(m_backgroundImage.width(), m_backgroundImage.height()));
        heatRect.setHeight(qMin(m_backgroundImage.width(), m_backgroundImage.height()));
        heatRect.moveCenter(m_backgroundImage.rect().center());
        break;
    case ModePanoramicMiniBullet:
        heatRect = m_backgroundImage.rect();
        break;
    default:
        break;
    }
    painter.save();
    switch (corridorMode) {
    case 1: {
        //顺时针旋转90度
        QMatrix matrix;
        matrix.rotate(90);
        heatMapImage = heatMapImage.transformed(matrix);
        break;
    }
    case 2: {
        //逆时针旋转90度
        QMatrix matrix;
        matrix.rotate(270);
        heatMapImage = heatMapImage.transformed(matrix);
        break;
    }
    default:
        break;
    }
    switch (imageRotation) {
    case 1: {
        //180度旋转
        QMatrix matrix;
        matrix.rotate(180);
        heatMapImage = heatMapImage.transformed(matrix);
        break;
    }
    case 2: {
        //水平翻转
        heatMapImage = heatMapImage.mirrored(true, false);
        break;
    }
    case 3: {
        //垂直翻转
        heatMapImage = heatMapImage.mirrored(false, true);
        break;
    }
    default:
        break;
    }
    painter.drawImage(heatRect, heatMapImage);
    painter.restore();

    //
    emit imageFinished(max, min, m_colorImage, m_backgroundImage);
    m_mutex.unlock();
}

void HeatMapThread::onSaveSpaceHeatMap(QString filePath)
{
    qDebug() << "----HeatMapThread::onSaveSpaceHeatMap----";
    qDebug() << "----filePath:" << filePath;

    QElapsedTimer timer;
    timer.start();

    bool ok = m_backgroundImage.save(filePath, "PNG");
    gMsMessage.syncFile();

    qDebug() << "----save took" << timer.elapsed() << "ms";
    qDebug() << "----result:" << ok;

    emit saveFinished(ok);
}
