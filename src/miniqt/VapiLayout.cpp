#include "VapiLayout.h"

#include "CameraInfo.h"
#include "vapi.h"
#include <QStringList>
#include <QtDebug>

VapiLayout::VapiLayout(QObject *parent) : QObject(parent) {
}

VapiLayout &VapiLayout::instance() {
    static VapiLayout self;
    return self;
}

void VapiLayout::vapiSetLayout(int row, int column) {
    LAYOUT_S stLayout;
    memset(&stLayout, 0, sizeof(stLayout));
    stLayout.enScreen = SCREEN_MAIN;

    int screenWidth, screenHeight;
    vapi_get_screen_res(stLayout.enScreen, &screenWidth, &screenHeight);

    int dw = screenWidth / column;
    int dh = screenHeight / row;

    FORMAT_S stFormat;
    memset(&stFormat, 0, sizeof(stFormat));
    stFormat.enType = ENC_TYPE_H265;
    stFormat.width  = 3840;
    stFormat.height = 2160;

    ZONE_S stZone;
    int    index = 0;
    for (int r = 0; r < row; r++) {
        for (int c = 0; c < column; ++c) {
            WIND_S &wind = stLayout.stWinds[index];

            memset(&stZone, 0, sizeof(stZone));
            stZone.enMode = ZONE_MODE_USER;
            stZone.x      = c * dw;
            stZone.y      = r * dh;
            stZone.w      = dw;
            stZone.h      = dh;

            wind.stDevinfo.id             = index;
            wind.stDevinfo.stBind.voutChn = index;
            wind.stDevinfo.stBind.vdecChn = index;
            wind.stDevinfo.stFormat       = stFormat;
            wind.enRatio                  = RATIO_VO_FULL;
            wind.stZone                   = stZone;

            index++;
        }
    }
    stLayout.winds_num = index;
    stLayout.enRefresh = REFRESH_CLEAR_VDEC;
    stLayout.enMode    = DSP_MODE_LIVE;
    stLayout.enScreen  = SCREEN_MAIN;

    vapi_clear_screen2(stLayout.enScreen);

    m_stLayout = stLayout;

    for (int i = 0; i < 64; ++i) {
        const auto &layout = m_mapLayoutInfo.value(i);
        if (layout.enable) {
            auto       *cameraInfo = CameraInfo::fromChannel(i);
            QStringList frame      = cameraInfo->frameSize().split("*");
            int         w          = 1920;
            int         h          = 1080;
            if (frame.size() == 2) {
                w = frame.at(0).toInt();
                h = frame.at(1).toInt();
            } else {
                qDebug() << QString("error frame size: %1, channel: %2, use 1920x1080 default").arg(cameraInfo->frameSize()).arg(i);
            }
            WIND_S &wind                   = m_stLayout.stWinds[i];
            wind.stDevinfo.stFormat.enType = (TYPE_E)cameraInfo->codec();
            wind.stDevinfo.stFormat.width  = w;
            wind.stDevinfo.stFormat.height = h;
            yu_vapi_add_one_win(m_stLayout.enScreen, m_stLayout.enMode, &wind);
        }
    }
}

void VapiLayout::updateLayout(int id, bool enable) {
    auto &layout = m_mapLayoutInfo[id];
    if (enable) {
        if (layout.enable) {

        } else {
            auto       *cameraInfo = CameraInfo::fromChannel(id);
            QStringList frame      = cameraInfo->frameSize().split("*");
            int         w          = 1920;
            int         h          = 1080;
            if (frame.size() == 2) {
                w = frame.at(0).toInt();
                h = frame.at(1).toInt();
            } else {
                qDebug() << QString("error frame size: %1, channel: %2, use 1920x1080 default").arg(cameraInfo->frameSize()).arg(id);
            }
            WIND_S &wind                   = m_stLayout.stWinds[id];
            wind.stDevinfo.stFormat.enType = (TYPE_E)cameraInfo->codec();
            wind.stDevinfo.stFormat.width  = w;
            wind.stDevinfo.stFormat.height = h;
            yu_vapi_add_one_win(m_stLayout.enScreen, m_stLayout.enMode, &wind);
            layout.enable = true;
        }
    } else {
        if (layout.enable) {
            yu_vapi_del_one_win(m_stLayout.enScreen, m_stLayout.enMode, id, m_stLayout.enRefresh);
            layout.enable = false;
        } else {
        }
    }
}
