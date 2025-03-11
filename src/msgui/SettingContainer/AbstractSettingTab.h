#ifndef ABSTRACTSETTINGTAB_H
#define ABSTRACTSETTINGTAB_H

#include <QMap>
#include "MsWidget.h"

extern "C" {
#include "msdb.h"
#include "msg.h"
}

class MessageReceive;

class AbstractSettingTab : public MsWidget {
    Q_OBJECT

public:
    explicit AbstractSettingTab(QWidget *parent = 0);

    static void setCurrentChannel(int channel);
    static int currentChannel();

    void back();
    void showWait();
    void showWait(QWidget *parent);
    void closeWait();

    virtual void initializeData();
    virtual void dealMessage(MessageReceive *message);

    virtual bool isCloseable();
    virtual bool isChangeable();
    virtual bool canAutoLogout();

signals:
    void sig_back();

protected slots:
    virtual void onLanguageChanged();

protected:
    static int s_currentChannel;
};

#endif // ABSTRACTSETTINGTAB_H
