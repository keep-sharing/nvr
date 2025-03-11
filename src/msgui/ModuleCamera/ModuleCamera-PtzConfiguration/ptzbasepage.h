#ifndef PTZBASEPAGE_H
#define PTZBASEPAGE_H

#include "MsWidget.h"

class MessageReceive;
class MsWaitting;
class PagePtzConfiguration;

class PtzBasePage : public MsWidget
{
    Q_OBJECT
public:
    explicit PtzBasePage(QWidget *parent = nullptr);

    virtual void initializeData() = 0;

    static int currentChannel();
    static void setCurrentChannel(int channel);

    void back();

protected:
    void showWait();
    void closeWait();

signals:

private:
    static int s_currentChannel;

    PagePtzConfiguration *m_ptzManager = nullptr;
    MsWaitting *m_waitting = nullptr;
};

#endif // PTZBASEPAGE_H
