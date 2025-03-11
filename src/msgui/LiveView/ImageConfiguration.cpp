#include "ImageConfiguration.h"
#include "ui_ImageConfiguration.h"
#include "centralmessage.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "SubControl.h"
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QtDebug>



ImageConfiguration::ImageConfiguration(QWidget *parent)
    : BaseDialog(parent)
    , ui(new Ui::ImageConfiguration)
{
    ui->setupUi(this);

    ui->horizontalSlider_brightness->setTextColor(QColor("#FFFFFF"));
    ui->horizontalSlider_contrast->setTextColor(QColor("#FFFFFF"));
    ui->horizontalSlider_saturation->setTextColor(QColor("#FFFFFF"));
    ui->horizontalSlider_sharpness->setTextColor(QColor("#FFFFFF"));
    ui->horizontalSlider_2d_dnr->setTextColor(QColor("#FFFFFF"));
    ui->horizontalSlider_noise->setTextColor(QColor("#FFFFFF"));
    ui->horizontalSlider_brightness->setShowValue(false);
    ui->horizontalSlider_contrast->setShowValue(false);
    ui->horizontalSlider_saturation->setShowValue(false);
    ui->horizontalSlider_sharpness->setShowValue(false);
    ui->horizontalSlider_2d_dnr->setShowValue(false);
    ui->horizontalSlider_noise->setShowValue(false);

    connect(ui->horizontalSlider_brightness, SIGNAL(valueChanged(int)), this, SLOT(onSetImageInfo()));
    connect(ui->horizontalSlider_contrast, SIGNAL(valueChanged(int)), this, SLOT(onSetImageInfo()));
    connect(ui->horizontalSlider_saturation, SIGNAL(valueChanged(int)), this, SLOT(onSetImageInfo()));
    connect(ui->horizontalSlider_sharpness, SIGNAL(valueChanged(int)), this, SLOT(onSetImageInfo()));
    connect(ui->horizontalSlider_2d_dnr, SIGNAL(valueChanged(int)), this, SLOT(onSetImageInfo()));
    connect(ui->horizontalSlider_noise, SIGNAL(valueChanged(int)), this, SLOT(onSetImageInfo()));

    connect(ui->horizontalSlider_brightness, SIGNAL(valueChanged(int)), ui->spinBox_brightness, SLOT(setValue(int)));
    connect(ui->horizontalSlider_contrast, SIGNAL(valueChanged(int)), ui->spinBox_contrast, SLOT(setValue(int)));
    connect(ui->horizontalSlider_saturation, SIGNAL(valueChanged(int)), ui->spinBox_saturation, SLOT(setValue(int)));
    connect(ui->horizontalSlider_sharpness, SIGNAL(valueChanged(int)), ui->spinBox_sharpness, SLOT(setValue(int)));
    connect(ui->horizontalSlider_2d_dnr, SIGNAL(valueChanged(int)), ui->spinBox_2d_dnr, SLOT(setValue(int)));
    connect(ui->horizontalSlider_noise, SIGNAL(valueChanged(int)), ui->spinBox_noise, SLOT(setValue(int)));

    connect(ui->spinBox_brightness, SIGNAL(valueChanged(int)), ui->horizontalSlider_brightness, SLOT(setValue(int)));
    connect(ui->spinBox_contrast, SIGNAL(valueChanged(int)), ui->horizontalSlider_contrast, SLOT(setValue(int)));
    connect(ui->spinBox_saturation, SIGNAL(valueChanged(int)), ui->horizontalSlider_saturation, SLOT(setValue(int)));
    connect(ui->spinBox_sharpness, SIGNAL(valueChanged(int)), ui->horizontalSlider_sharpness, SLOT(setValue(int)));
    connect(ui->spinBox_2d_dnr, SIGNAL(valueChanged(int)), ui->horizontalSlider_2d_dnr, SLOT(setValue(int)));
    connect(ui->spinBox_noise, SIGNAL(valueChanged(int)), ui->horizontalSlider_noise, SLOT(setValue(int)));

    //0.5秒内连续设置多次数值，只发送最后一次
    m_sendTimer = new QTimer(this);
    m_sendTimer->setSingleShot(true);
    m_sendTimer->setInterval(500);
    connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(onSendTimer()));

    //
    ui->label_title->installEventFilter(this);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

ImageConfiguration::~ImageConfiguration()
{
    delete ui;
}

void ImageConfiguration::showImageInfo(int channel, const QRect &videoGeometry)
{
    QRect screenRect = SubControl::instance()->logicalMainScreenGeometry();
    QPoint p(videoGeometry.left() - ui->widget_image->width(), videoGeometry.top());
    if (p.x() < screenRect.left()) {
        p.setX(videoGeometry.right());
    }
    if (p.y() + ui->widget_image->height() > screenRect.bottom()) {
        p.setY(screenRect.bottom() - ui->widget_image->height());
    }
    if (p.x() >= screenRect.right()) {
        p.setX(videoGeometry.x());
    }
    if (p.y() >= screenRect.bottom()) {
        p.setY(videoGeometry.y());
    }
    ui->widget_image->move(p);
    //
    m_currentChannel = channel;

    camera cam;
    memset(&cam, 0, sizeof(camera));
    read_camera(SQLITE_FILE_NAME, &cam, channel);
    bool enable = cam.camera_protocol != IPC_PROTOCOL_RTSP;
    ui->horizontalSlider_brightness->setEnabled(enable);
    ui->spinBox_brightness->setEnabled(enable);
    ui->horizontalSlider_contrast->setEnabled(enable);
    ui->spinBox_contrast->setEnabled(enable);
    ui->horizontalSlider_saturation->setEnabled(enable);
    ui->spinBox_saturation->setEnabled(enable);
    ui->horizontalSlider_sharpness->setEnabled(enable);
    ui->spinBox_sharpness->setEnabled(enable);
    ui->horizontalSlider_2d_dnr->setEnabled(enable);
    ui->spinBox_2d_dnr->setEnabled(enable);
    ui->horizontalSlider_noise->setEnabled(enable);
    ui->spinBox_noise->setEnabled(enable);
    ui->pushButton_default->setEnabled(enable);
    if (!enable) {
        return;
    }

        ui->label_noiseReductionLevel->setVisible(true);
        ui->horizontalSlider_noise->setVisible(true);
        ui->spinBox_noise->setVisible(true);
    

    sendMessage(REQUEST_FLAG_GET_IMAGEPARAM, (void *)&channel, sizeof(int));
}

void ImageConfiguration::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IMAGEPARAM:
        ON_RESPONSE_FLAG_GET_IMAGEPARAM(message);
        break;
    }
}

void ImageConfiguration::ON_RESPONSE_FLAG_GET_IMAGEPARAM(MessageReceive *message)
{
    if (message->data == nullptr) {
        qWarning() << QString("ON_RESPONSE_FLAG_GET_IMAGEPARAM, data is null.");
        return;
    }
    struct resp_get_imageparam *image = (struct resp_get_imageparam *)message->data;
    if (image->chanid != m_currentChannel) {
        qWarning() << QString("ON_RESPONSE_FLAG_GET_IMAGEPARAM, result channel(%1) != current channel(%2)").arg(image->chanid).arg(m_currentChannel);
        return;
    }

    char version[50];
    //get_channel_version_name(m_currentChannel, version, sizeof(version));
    MsCameraVersion cameraVersion(version);
    if (cameraVersion > MsCameraVersion (7, 72) ) {
        ui->label_2d_dnr->setVisible(true);
        ui->horizontalSlider_2d_dnr->setVisible(true);
        ui->spinBox_2d_dnr->setVisible(true);
    } else {
        ui->label_2d_dnr->setVisible(false);
        ui->horizontalSlider_2d_dnr->setVisible(false);
        ui->spinBox_2d_dnr->setVisible(false);
    }

    m_sendChange = false;
    ui->horizontalSlider_brightness->setValue(toPercentValue(image->brightness));
    ui->horizontalSlider_contrast->setValue(toPercentValue(image->contrast));
    ui->horizontalSlider_saturation->setValue(toPercentValue(image->saturation));
    ui->horizontalSlider_sharpness->setValue(toPercentValue(image->sharpness));
    ui->horizontalSlider_2d_dnr->setValue(image->nf2level);
    ui->horizontalSlider_noise->setValue(image->nflevel);
    m_sendChange = true;

    QString strDebug = QString("RESPONSE_FLAG_GET_IMAGEPARAM, channel: %1\n").arg(m_currentChannel);
    strDebug.append(QString("brightness: %1(%2)\n").arg(ui->spinBox_brightness->value()).arg(image->brightness));
    strDebug.append(QString("contrast: %1(%2)\n").arg(ui->spinBox_contrast->value()).arg(image->contrast));
    strDebug.append(QString("saturation: %1(%2)\n").arg(ui->spinBox_saturation->value()).arg(image->saturation));
    strDebug.append(QString("sharpness: %1(%2)\n").arg(ui->spinBox_sharpness->value()).arg(image->sharpness));
    strDebug.append(QString("2D DNR: %1(%2)\n").arg(ui->spinBox_2d_dnr->value()).arg(image->nf2level));
    strDebug.append(QString("3D DNR: %1(%2)").arg(ui->spinBox_noise->value()).arg(image->nflevel));
    qDebug() << strDebug;
}

void ImageConfiguration::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    showMaximized();

    BaseDialog::showEvent(event);
}

bool ImageConfiguration::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->label_title) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            m_titlePressed = true;
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            m_titlePressedDistance = mouseEvent->globalPos() - ui->widget_image->pos();
            ui->label_title->setCursor(Qt::ClosedHandCursor);
            break;
        }
        case QEvent::MouseButtonRelease: {
            m_titlePressed = false;
            ui->label_title->setCursor(Qt::OpenHandCursor);
            break;
        }
        case QEvent::MouseMove: {
            if (m_titlePressed) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                ui->widget_image->move(mouseEvent->globalPos() - m_titlePressedDistance);
            }
            break;
        }
        default:
            break;
        }
    }
    return BaseDialog::eventFilter(object, event);
}

void ImageConfiguration::mousePressEvent(QMouseEvent *event)
{
    if (!ui->widget_image->geometry().contains(event->pos())) {
        on_pushButton_close_clicked();
    }
    BaseDialog::mousePressEvent(event);
}

void ImageConfiguration::escapePressed()
{
    on_pushButton_close_clicked();
}

bool ImageConfiguration::isMoveToCenter()
{
    return false;
}

bool ImageConfiguration::isAddToVisibleList()
{
    return true;
}

int ImageConfiguration::toPercentValue(int value)
{
    return qRound(value * 100 / 255.0);
}

int ImageConfiguration::fromPercentValue(int value)
{
    return qRound(value * 255 / 100.0);
}

void ImageConfiguration::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("LIVEVIEW/20076", "Image"));
    ui->label_brightness->setText(GET_TEXT("LIVEVIEW/20077", "Brightness:"));
    ui->label_contrast->setText(GET_TEXT("LIVEVIEW/20078", "Contrast:"));
    ui->label_saturation->setText(GET_TEXT("LIVEVIEW/20079", "Saturation:"));
    ui->label_sharpness->setText(GET_TEXT("LIVEVIEW/20081", "Sharpness:"));

    ui->pushButton_default->setText(GET_TEXT("COMMON/1050", "Default"));
    ui->pushButton_close->setText(GET_TEXT("PTZDIALOG/21005", "Close"));
}

void ImageConfiguration::onSetImageInfo()
{
    if (!m_sendChange) {
        return;
    }

    m_sendTimer->start();
}

void ImageConfiguration::onSendTimer()
{
    struct req_set_imageparam image;
    memset(&image, 0, sizeof(image));
    image.chanid = m_currentChannel;
    image.brightness = fromPercentValue(ui->spinBox_brightness->value());
    image.contrast = fromPercentValue(ui->spinBox_contrast->value());
    image.saturation = fromPercentValue(ui->spinBox_saturation->value());
    image.sharpness = fromPercentValue(ui->spinBox_sharpness->value());
    image.nf2level = ui->spinBox_2d_dnr->value();
    image.nflevel = ui->spinBox_noise->value();
    sendMessageOnly(REQUEST_FLAG_SET_CAMERA_IMAGEPARAM, (void *)&image, sizeof(image));

    QString strDebug = QString("REQUEST_FLAG_SET_CAMERA_IMAGEPARAM, channel: %1\n").arg(m_currentChannel);
    strDebug.append(QString("brightness: %1(%2)\n").arg(ui->spinBox_brightness->value()).arg(image.brightness));
    strDebug.append(QString("contrast: %1(%2)\n").arg(ui->spinBox_contrast->value()).arg(image.contrast));
    strDebug.append(QString("saturation: %1(%2)\n").arg(ui->spinBox_saturation->value()).arg(image.saturation));
    strDebug.append(QString("sharpness: %1(%2)\n").arg(ui->spinBox_sharpness->value()).arg(image.sharpness));
    strDebug.append(QString("2D DNR: %1(%2)\n").arg(ui->spinBox_2d_dnr->value()).arg(image.nf2level));
    strDebug.append(QString("3D DNR: %1(%2)").arg(ui->spinBox_noise->value()).arg(image.nflevel));
    qDebug() << strDebug;
}

void ImageConfiguration::on_pushButton_default_clicked()
{
    ui->horizontalSlider_brightness->setValue(50);
    ui->horizontalSlider_contrast->setValue(50);
    ui->horizontalSlider_saturation->setValue(50);
    ui->horizontalSlider_sharpness->setValue(50);
    ui->horizontalSlider_2d_dnr->setValue(50);
    ui->horizontalSlider_noise->setValue(50);
}

void ImageConfiguration::on_pushButton_close_clicked()
{
    close();
}
