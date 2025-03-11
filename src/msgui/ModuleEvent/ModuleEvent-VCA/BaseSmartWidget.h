#ifndef BASESMARTWIDGET_H
#define BASESMARTWIDGET_H

#include "MsWidget.h"
#include <QEventLoop>
#include "channelcopydialog.h"

class SmartEvent;
class MessageReceive;
class ms_vca_license;

class BaseSmartWidget : public MsWidget {
    Q_OBJECT

public:
    explicit BaseSmartWidget(QWidget *parent = 0);

    static int currentChannel();
    static void setCurrentChannel(int channel);

    void showWait();
    void closeWait();

    bool isChannelEnable(int channel);

    int vcaType();
    bool isVcaSupport();
    int waitForCheckVcaSupport(int channel);

    int waitForCheckFisheeyeSupport(int channel);

    void dealGlobalMessage(MessageReceive *message);

    virtual void initializeData(int channel) = 0;
    virtual void saveData() = 0;
    virtual void copyData() = 0;
    virtual void clearCopyInfo() = 0;

protected:
    bool isSupportDetectionObject(int channel);

    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VAC_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_LICENSE(MessageReceive *message);

    void setApplyButtonEnable(bool enable);

    void showNotConnectedMessage();
    void showNotSupportMessage();

    void showDataError();

signals:
    void showMessage(QString message);
    void hideMessage();

public slots:

protected:
    QEventLoop m_eventLoop;

    bool m_isConnected = false;
    bool m_isSupported = -1;

private:
    static int s_currentChannel;

    SmartEvent *m_smartEvent = nullptr;

    bool m_isVcaSupport = false;
    bool m_isLicenseVaild = false;

    int m_vcaType = 0;
};

#endif // BASESMARTWIDGET_H
