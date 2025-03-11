#pragma once

#include "MsWidget.h"

class AdvancedSettings;
class MessageReceive;

class AbstractAdvancedSettingsPage : public MsWidget {
    Q_OBJECT

public:
    explicit AbstractAdvancedSettingsPage(QWidget *parent = nullptr);

    void setDrawWidget(QWidget *widget);
    void back();
    void setChannelConnected(bool connected);
    bool isChannelConnected();
    int channel();
    void setChannel(int);
    virtual void hideDrawWidget();
    virtual void initializeData() = 0;

signals:
    void sig_back();
    void showWait();
    void closeWait();

private:
    AdvancedSettings *m_advancedSettings = nullptr;
    bool m_isConnected = false;
    int m_channel = -1;
};
