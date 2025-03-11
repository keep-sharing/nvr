#include "BaseSmartWidget.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "smartevent.h"

extern "C" {
#include "msg.h"

}

int BaseSmartWidget::s_currentChannel = 0;

BaseSmartWidget::BaseSmartWidget(QWidget *parent)
    : MsWidget(parent)
{
    m_smartEvent = static_cast<SmartEvent *>(parent);
}

int BaseSmartWidget::currentChannel()
{
    return s_currentChannel;
}

void BaseSmartWidget::setCurrentChannel(int channel)
{
    s_currentChannel = channel;
}

void BaseSmartWidget::showWait()
{
    //MsWaitting::showGlobalWait();
}

void BaseSmartWidget::closeWait()
{
    //MsWaitting::closeGlobalWait();
}

bool BaseSmartWidget::isChannelEnable(int channel)
{
    return LiveView::instance()->isChannelConnected(channel);
}

bool BaseSmartWidget::isVcaSupport()
{
    return m_isVcaSupport && m_isLicenseVaild;
}

int BaseSmartWidget::vcaType()
{
    return m_vcaType;
}

int BaseSmartWidget::waitForCheckVcaSupport(int channel)
{
    Q_UNUSED(channel)
    return 0;
}

int BaseSmartWidget::waitForCheckFisheeyeSupport(int channel)
{
    Q_UNUSED(channel)
    return 0;
}

void BaseSmartWidget::dealGlobalMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_VAC_SUPPORT:
        ON_RESPONSE_FLAG_GET_VAC_SUPPORT(message);
        break;
    case RESPONSE_FLAG_GET_VCA_LICENSE:
        ON_RESPONSE_FLAG_GET_VCA_LICENSE(message);
        break;
    default:
        break;
    }
}

/**
 * @brief BaseSmartWidget::isSupportDetectionObject
 * 是否支持人车属性
 * @param channel
 * @return
 */
bool BaseSmartWidget::isSupportDetectionObject(int channel)
{
    MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(channel);
    if (cameraVersion.chipType() == 4 && cameraVersion.chipVersion() == 5 && cameraVersion >= MsCameraVersion(7, 77)) {
        return true;
    }
    //TODO: LiuHuanyu 2021-08-18, 改用接口判断
    //【vca：nvr的vca模块未兼容全景/ptz系列的人车属性】https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001051926
    if (cameraVersion.chipType() == 3) {
        switch (cameraVersion.chipVersion()) {
        case 1:
        case 3:
            return true;
        }
    }
    //Sigma 51.7.0.76-r11开始支持人车属性
    if (cameraVersion.chipType() == 5 && cameraVersion.chipVersion() == 1 && cameraVersion >= MsCameraVersion(7, 76, 11)) {
        return true;
    }
    //联咏新平台
    if (cameraVersion.chipType() == 6 && cameraVersion.chipVersion() == 1) {
      return true;
    }

    return false;
}

/**
 * @brief BaseSmartWidget::respVcaSupport
 * @param data vca_support: 1：支持，0：不支持，-1：通道断开
 */
void BaseSmartWidget::ON_RESPONSE_FLAG_GET_VAC_SUPPORT(MessageReceive *message)
{
    ms_vca_support_info *info = (ms_vca_support_info *)message->data;
    m_isVcaSupport = (info->vca_support == 1);
    m_vcaType = info->vca_type;
    qMsDebug() << QString("channel: %1, vca_support: %2, type: %3").arg(info->chanid).arg(info->vca_support).arg(info->vca_type);

    m_eventLoop.exit();
}

void BaseSmartWidget::ON_RESPONSE_FLAG_GET_VCA_LICENSE(MessageReceive *message)
{
    ms_vca_license *info = (ms_vca_license *)message->data;
    m_isLicenseVaild = (info->vca_license_status == 1);
    qMsDebug() << QString("channel: %1, vca_license_status: %2").arg(info->chanid).arg(info->vca_license_status);

    m_eventLoop.exit();
}

void BaseSmartWidget::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    Q_UNUSED(message)
}

void BaseSmartWidget::setApplyButtonEnable(bool enable)
{
    m_smartEvent->setApplyButtonEnable(enable);
}

void BaseSmartWidget::showNotConnectedMessage()
{
    //closeWait();
    emit showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
}

void BaseSmartWidget::showNotSupportMessage()
{
    //closeWait();
    emit showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
}

/**
 * @brief BaseSmartWidget::showDataError
 * 容错处理，一般是用不到的
 */
void BaseSmartWidget::showDataError()
{
    //closeWait();
    ShowMessageBox(this, "Get data error, data is nullptr!");
}
