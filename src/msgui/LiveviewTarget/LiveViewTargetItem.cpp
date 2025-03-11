#include "LiveViewTargetItem.h"
#include "ui_LiveViewTargetItem.h"
#include "DisplaySetting.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "TargetInfoManager.h"
#include <QMouseEvent>
#include <QPainter>
#include <QtDebug>

LiveViewTargetItem::LiveViewTargetItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LiveViewTargetItem)
{
    ui->setupUi(this);

    m_plateIcon = new ToolButtonFace(ui->pageAnpr);
    m_plateIcon->setIconSize(QSize(32, 24));
    m_plateIcon->move(135, 30);
    m_plateIcon->hide();

    clearItemInfo();

    connect(qMsNvr, SIGNAL(liveViewBorderChanged(bool, QColor)), this, SLOT(onBorderStateChanged(bool, QColor)));
}

LiveViewTargetItem::~LiveViewTargetItem()
{
    delete ui;
}

QRect LiveViewTargetItem::globalGeometry() const
{
    return QRect(mapToGlobal(QPoint(0, 0)), size());
}

void LiveViewTargetItem::setIndex(int index)
{
    m_index = index;

    ui->widgetImage->setIndex(m_index);
}

void LiveViewTargetItem::setTempHide(bool hide, bool hideBorder)
{
    m_isTempHide = hide;
    m_isHideBorder = hideBorder;
    ui->widgetLeft->setVisible(!m_isTempHide);
    ui->stackedWidget->setVisible(!m_isTempHide);
    update();
}

int LiveViewTargetItem::channel() const
{
    return m_channel;
}

QString LiveViewTargetItem::plate() const
{
    if (m_infoType == TargetInfo::TARGET_ANPR) {
        return m_plate;
    } else {
        return QString();
    }
}

QDateTime LiveViewTargetItem::anprTime() const
{
    return QDateTime::fromString(m_time, "yyyy-MM-dd HH:mm:ss");
}

void LiveViewTargetItem::updateItemInfo()
{
    gTargetInfoManager.lock();
    TargetInfo *info = gTargetInfoManager.getTargetInfo(m_index);
    if (info) {
        if (!ui->widgetLeft->isVisible()) {
            ui->widgetLeft->show();
        }
        if (!ui->stackedWidget->isVisible()) {
            ui->stackedWidget->show();
        }

        m_infoType = info->type();
        m_channel = info->channel();
        m_time = info->timeString();

        ui->labelChannel->setElidedText(QString("%1: %2").arg(GET_TEXT("CHANNELMANAGE/30008", "Channel")).arg(qMsNvr->channelName(m_channel)));
        ui->labelChannel->setToolTip(qMsNvr->channelName(m_channel));
        ui->labelTime->setElidedText(m_time);
        ui->widgetImage->update();

        switch (info->type()) {
        case TargetInfo::TARGET_ANPR: {
            ui->stackedWidget->setCurrentWidget(ui->pageAnpr);
            TargetInfoAnpr *anprInfo = static_cast<TargetInfoAnpr *>(info);

            m_plate = anprInfo->licenseString();
            QString anprType = anprInfo->plateTypeString();
            if (anprType == QString(PARAM_MS_ANPR_TYPE_BLACK)) {
                m_plateIcon->setIcon(QIcon(":/liveview/liveview/car_red.png"));
                m_plateIcon->show();
            } else if (anprType == QString(PARAM_MS_ANPR_TYPE_WHITE)) {
                m_plateIcon->setIcon(QIcon(":/liveview/liveview/car_green.png"));
                m_plateIcon->show();
            } else {
                m_plateIcon->hide();
            }
            ui->labelLicense->setElidedText(QString("%1: %2").arg(GET_TEXT("ANPR/103035", "License Plate")).arg(anprInfo->licenseString()));
            ui->labelPlateType->setElidedText(QString("%1: %2").arg(GET_TEXT("ANPR/103041", "Plate Type")).arg(anprInfo->plateTypeString()));
            ui->labelPlateColor->setElidedText(QString("%1: %2").arg(GET_TEXT("ANPR/103099", "Plate Color")).arg(anprInfo->plateColorString()));
            ui->labelVehicleType->setElidedText(QString("%1: %2").arg(GET_TEXT("ANPR/103100", "Vehicle Type")).arg(anprInfo->vehicleTypeString()));
            ui->labelVehicleBrand->setElidedText(QString("%1: %2").arg(GET_TEXT("ANPR/169006", "Vehicle Brand")).arg(anprInfo->vehicleBrandString()));
            ui->labelVehicleColor->setElidedText(QString("%1: %2").arg(GET_TEXT("ANPR/103101", "Vehicle Color")).arg(anprInfo->vehicleColorString()));
            ui->labelSpeed->setElidedText(QString("%1: %2").arg(GET_TEXT("PTZDIALOG/21020", "Speed")).arg(anprInfo->speedString()));
            ui->labelDirection->setElidedText(QString("%1: %2").arg(GET_TEXT("ANPR/103022", "Direction")).arg(anprInfo->directionString()));
            break;
        }
        case TargetInfo::TARGET_VCA: {
            ui->stackedWidget->setCurrentWidget(ui->pageVca);
            TargetInfoVca *vcaInfo = static_cast<TargetInfoVca *>(info);

            QString vcaEventText = vcaInfo->vcaEventString();
            ui->labelVcaEvent->setElidedText(QString("%1: %2").arg(GET_TEXT("TARGETMODE/103209", "Event")).arg(vcaEventText));
            ui->labelVcaEvent->setToolTip(vcaEventText);
            QString vcaObjectText = vcaInfo->vcaObjectString();
            ui->labelVcaObject->setElidedText(QString("%1: %2").arg(GET_TEXT("TARGETMODE/103200", "Detection Object")).arg(vcaObjectText));
            ui->labelVcaObject->setToolTip(vcaObjectText);
            break;
        }
        case TargetInfo::TARGET_FACE: {
            ui->stackedWidget->setCurrentWidget(ui->pageFace);
            ui->labelFaceDetection->setText(QString("%1: %2").arg(GET_TEXT("TARGETMODE/103209", "Event")).arg(GET_TEXT("FACE/141000", "Face Detection")));
            TargetInfoFace *faceInfo = static_cast<TargetInfoFace *>(info);
            faceInfo->updateMaleInfo(ui->toolButtonMale);
            faceInfo->updateAgeInfo(ui->toolButtonAge);
            faceInfo->updateGlassesInfo(ui->toolButtonGlasses);
            faceInfo->updateMaskInfo(ui->toolButtonFaceMask);
            faceInfo->updateCapInfo(ui->toolButtonHat);
            break;
        }
        default:
            break;
        }
    }
    gTargetInfoManager.unlock();
}

void LiveViewTargetItem::clearItemInfo()
{
    ui->widgetLeft->hide();
    ui->stackedWidget->hide();
    m_channel = -1;
}

bool LiveViewTargetItem::hasInfo() const
{
    return channel() >= 0;
}

int LiveViewTargetItem::infoType() const
{
    return m_infoType;
}

void LiveViewTargetItem::setNetworkFocus(bool focus)
{
    m_isNetworkFocus = focus;
    update();
}

void LiveViewTargetItem::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(m_borderColor, 0.5));
    if (m_isHideBorder || !m_borderVisible) {

    } else {
        painter.drawLine(rect().topLeft(), rect().topRight());
    }

    if (m_isNetworkFocus) {
        painter.save();
        painter.setPen(QPen(Qt::white, 4));
        painter.drawRect(rect());
        painter.restore();
    }
}

void LiveViewTargetItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit itemClicked(m_index);
    } else if (event->button() == Qt::RightButton) {
        emit itemContextMenuRequested(m_index, event->globalPos());
    }
}

void LiveViewTargetItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void LiveViewTargetItem::onBorderStateChanged(bool visible, const QColor &color)
{
    m_borderVisible = visible;
    m_borderColor = color;
    update();
}
