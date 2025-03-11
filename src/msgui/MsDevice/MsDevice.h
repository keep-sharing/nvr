#ifndef MSDEVICE_H
#define MSDEVICE_H

#include "AlarmKey.h"
#include "MsCameraModel.h"
#include "MsCameraVersion.h"
#include "MsGlobal.h"
#include "MsObject.h"
#include <QColor>
#include <QMap>
#include <QMutex>
#include <QPixmap>
#include <QVariant>

extern "C" {
#include "msdb.h"
#include "msfs_bkp.h"
#include "msg.h"
#include "msoem.h"

#ifdef MS_FISHEYE_SOFT_DEWARP
#include "fisheye_player.h"
#endif
}

Q_DECLARE_METATYPE(resq_get_ipcdev)

#define qMsNvr MsDevice::instance()

const int InvalidValue = -12345;

#define MAX_SEARCH_BACKUP_COUNT BKP_MAX_RESULT_NUM

namespace MsQt {
enum FISHEYE_TRANSFER {
    FISHEYE_MULTI = 0,
    FISHEYE_BUNDLE = 1
};

enum FISHEYE_INSTALL_MODE {
    FISHEYE_INSTALL_CEILING = 0,
    FISHEYE_INSTALL_WALL = 1,
    FISHEYE_INSTALL_FLAT = 2
};

enum FISHEYE_DISPLAY_MODE {
    FISHEYE_DISPLAY_1O = 0,
    FISHEYE_DISPLAY_1P = 1,
    FISHEYE_DISPLAY_2P = 2,
    FISHEYE_DISPLAY_4R = 4,
    FISHEYE_DISPLAY_1O3R = 5,
    FISHEYE_DISPLAY_1P3R = 6,
    FISHEYE_DISPLAY_1O1P3R = 7
};
}

class MsDevice : public MsObject {
    Q_OBJECT

public:
    //NVR 平台
    enum Type {
        HI_NVR
    };

    explicit MsDevice(QObject *parent = nullptr);
    ~MsDevice();

    static MsDevice *instance();
    static void deleteInstance();

    static QPixmap *s_backgroundPixmap;

    static void no_resource_callback(int winid, int streamid, int bNorsc);

    void readyToQuit();

    //
#ifdef MS_FISHEYE_SOFT_DEWARP
    static FisheyePlayer *s_fisheyeHandle;
    static void fisheye_handle_callback(void *handle);
    static QMutex s_fisheyeMutex;
    static void fisheyeHandleLock();
    static void fisheyeHandleUnlock();
#endif

    // 4k
    virtual bool isSupportResolution_4k_60Hz() const;
    // 是否支持raid
    virtual bool isSupportRaid() const;
    //
    virtual bool isSupportBestDecoding() const;
    // 是否支持双码流录像
    virtual bool isSupportDualStreamVideoRecording() const;
    // 是否有物理关机按键
    virtual bool isSupportPhysicalShutdown() const;
    //
    virtual bool isSupportESataFunction() const;

    //
    device_info deviceInfo() const;
    //display
    void readDisplayInfo();
    void writeDisplayInfo(struct display *info);
    const display &displayInfo() const;
    void writeDisplayColor(int index, const QColor &color);
    //
    network networkInfo() const;
    QString deviceName() const;

    //platform
    MsDevice::Type platform();

    //3798
    bool is3798() const;
    //3536
    bool is3536() const;
    //3536g
    bool is3536g() const;
    //3536c
    bool is3536c() const;
    //3536a
    bool is3536a() const;

    bool is5016() const;
    bool is1004() const;

    bool isnt98323() const;
    bool isnt98633() const;

    bool is8000Series() const;
    bool is7000Series() const;
    bool is5000Series() const;
    bool is1000Series() const;

    int maxChannel() const;
    int maxAlarmOutput() const;
    int maxAlarmInput() const;
    quint64 channelMaskValue() const;
    quint64 channelMaskValue(int channel) const;

    bool isSupportEventSnapshot() const;
    bool isSupportContinuousSnapshot() const;

    DisplayMode displayMode();
    //
    bool isTargetMode();
    void setTargetMode(bool mode);
    bool isOccupancyMode();
    void setOccupancyMode(bool mode);

    //boot wizard
    bool isBootWizardEnable();
    void setBootWizardEnable(bool enable);

    //boot authentication
    bool isLocalAuthentication();
    void setLocalAuthentication(bool enable);

    //menu authentication
    bool isMenuAuthentication();
    void setMenuAuthentication(bool enable);

    //bit rate
    QList<int> bitRateList() const;

    //oem
    bool isOEM() const;
    OEM_TYPE OEMType() const;
    bool isMilesight() const;

    //model
    QString model() const;

    //hardware
    QString hardwareVersion() const;
    double hardwareVersionEx() const; // V2.1 --> 2.1

    //software
    QString softwareVersion() const;

    //camera
    QMap<int, camera> cameraMap();
    QList<int> disabledCameraList();
    void readDatabaseCameras();
    void readDatabaseCamera(struct camera *cam, int channel);
    void writeDatabaseCamera(struct camera *cam);
    struct camera databaseCamera(int channel);
    int nextUnusedChannel();
    int checkSameChannelExist(int channel, const QString &ip, int port, int channel_id);
    int checkSameDDNSExist(int channel, const QString &ddns, int channel_id);
    int checkSameRTSPExist(int channel, const QString &rtsp, int port);
    bool isChannelConnected(int channel);
    void updateCameraInfo(resq_get_ipcdev *ipc_array, int count);
    resq_get_ipcdev cameraInfo(int channel);
    bool isFisheye(int channel);
    bool isMsCamera(int channel);
    MsCameraVersion cameraVersion(int channel);
    bool isCameraVersionEqualOrGreaterThan(int channel, const MsCameraVersion &version);
    void updateCameraRealModel(resp_cam_model_info *model);
    void updateSingleCameraRealModel(int channel, CAM_MODEL_INFO *model);
    MsCameraModel cameraRealModel(int channel);
    bool isSigma(int channel);
    bool isPtzBullet(int channel);
    bool isAmba(int channel);
    bool isNT(int channel);
    bool isHisi(int channel);

    //osd
    QMap<int, osd> osdMap();
    QString osdName(int channel) const;
    void setOSDName(int channel, const QString &name);

    void updateChannelsName();
    QString channelName(int channel) const;

    //protocol
    void readProtocolList();
    QString protocolName(int id);
    QString protocolTypeString(int type);
    QMap<int, ipc_protocol> protocolMap();

    //mac
    QStringList macList() const;

    //poe
    bool isPoe() const;
    int poeCount() const;

    //user
    QString currentUserName() const;

    //audio
    void openLiveviewAudio(int channel);
    void openPlaybackAudio(int channel);
    void closeLiveviewAudio();
    void closeLiveviewAudio(int channel);
    void closePlaybackAudio();
    int liveviewAudioChannel() const;
    int playbackAudioChannel() const;
    bool isAudioOpen() const;
    bool isLiveviewAudioOpen() const;
    void closeAudio();
    void closeAudio(int channel);

    //是否支持对讲
    bool isSupportTalkback() const;
    bool isCamTalkbackOpen() const;
    int talkbackChannel() const;
    void openTalkback(int channel);
    void closeTalkback();
    void closeTalkback(int channel);

    int get_mac_addr(char mac0[32], char mac1[32]);

    //rs485
    bool hasRs485() const;

    int maxDiskPort() const;

    //
    bool isSlaveMode();

    //anpr
    bool isSupportTargetMode() const;
    bool isSupportSnapshot() const;
    bool isSupportAudio() const;

    //face
    bool isSupportFaceDetection() const;

    //
    bool isVaildValue(int value);

    bool isSameChannelCanExist();

    //switch screen
    bool isQuickSwitchScreenEnable() const;
    void setQuickSwitchScreenEnable(bool enable);
    int multiScreenSupport() const;

    //
    QMap<AlarmKey, QString> allAlarmoutNames() const;
    QMap<AlarmKey, QString> allAlarminNames() const;
    //camera alarmin name
    QMap<AlarmKey, QString> allChannelAlarmoutNames() const;
    QMap<AlarmKey, QString> allChannelAlarminNames() const;

    //mouse accel
    void initializeMouseAccel();

    bool isNoResource(int winid);
    bool hasNoResource(SCREEN_E screen);
    void clearNoResource();
    void clearNoResource(SCREEN_E screen);

    //log
    void writeLog(const camera &cam, int sub_type, int parameter_type);
    void writeLog(int sub_type);

    //port
    bool isPortUsed(int port) const;

    //
    void mallocTrimLater(int msec);

signals:
    /**display begin**/
    void displayPlayModeChanged(int mode);
    void displayColorChanged(const QColor &color);
    void displayStreamInfoChanged(int value);
    void displayChannelNameChanged(int value);
    void displayBorderlineChanged(int value);
    void displayPageInfoChanged(int value);
    void displayTimeInfoChanged(int value);
    void displayEventDetectionChanged(int event);

    void liveViewBorderChanged(bool visible, const QColor &color);
    /**display end**/

    void noResourceChanged(int winid, int bNoResource);
    void fisheyeHandleChanged();

public slots:
    //reboot
    void reboot();
    void rebootLater(int msec);

    //shutdown
    void shutdown();
    void shutdownLater(int msec);

    //
    void onMallocTrim();

private slots:

private:
    static MsDevice *s_msDevice;

    static QMap<int, bool> s_noResourceMap;

    device_info m_deviceInfo;
    device_info m_deviceInfoOEM;
    network m_networkInfo;
    display m_displayInfo;
    SCREEN_E m_screen = SCREEN_MAIN;

    QMap<int, camera> m_cameraMap;
    struct camera *m_camera_array = nullptr;
    QMap<int, ipc_protocol> m_protocolMap;

    QMap<int, QString> m_channelNameMap;

    int m_poeCount = 0;

    //user
    QString m_currentUserName;

    //
    int m_sequenceInterval = InvalidValue;

    //audio
    int m_liveviewAudioChannel = -1;
    int m_playbackAudioChannel = -1;
    int m_talkbackChannel = -1;

    //
    QMap<int, resq_get_ipcdev> m_cameraInfoMap;
    //
    resp_cam_model_info m_cameraRealModel;
};

#endif // MSDEVICE_H
