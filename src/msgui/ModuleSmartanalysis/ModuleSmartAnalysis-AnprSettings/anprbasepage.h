#ifndef ANPRBASEPAGE_H
#define ANPRBASEPAGE_H

#include "MsWidget.h"
#include <QGraphicsItem>

extern "C" {
#include "msg.h"
}

class PageAnprSettings;
class MessageReceive;

class AnprBasePage : public MsWidget {
    Q_OBJECT
public:
    explicit AnprBasePage(QWidget *parent = nullptr);

    static void setAnprSupportInfo(ms_lpr_support_info *lpr_support_info);
    static bool isAnprSupport();
    static int anprVersion();

    static void setCurrentChannel(int channel);
    static int currentChannel();

    static void setChannelConnected(bool connected);
    static bool isChannelConnected();

    void setDrawWidget(QWidget *widget);
    void addGraphicsItem(QGraphicsItem *item);
    void back();

    virtual void initializeData(int channel) = 0;

signals:
    void sig_back();

public slots:
    void showWait();
    void closeWait();

private:
    static ms_lpr_support_info s_lpr_support_info;
    static int s_currentChannel;
    static bool s_isConnected;

    PageAnprSettings *m_anprSetting = nullptr;
};

#endif // ANPRBASEPAGE_H
