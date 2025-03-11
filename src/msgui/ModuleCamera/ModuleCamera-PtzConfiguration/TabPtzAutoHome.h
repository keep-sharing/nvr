#ifndef TABPTZAUTOHOME_H
#define TABPTZAUTOHOME_H

#include "ptzbasepage.h"
#include "QEventLoop"

extern "C" {
#include "msg.h"
}

namespace Ui {
class TabPtzAutoHome;
}

class TabPtzAutoHome : public PtzBasePage {
    Q_OBJECT

public:
    explicit TabPtzAutoHome(QWidget *parent = nullptr);
    ~TabPtzAutoHome();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void setSettingEnable(bool enable);
    void ON_RESPONSE_FLAG_GET_IPC_PTZ_AUTO_HOME(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_PTZ_AUTO_HOME(MessageReceive *message);

private slots:
    void onLanguageChanged();
    void onChannelGroupClicked(int channel);
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

    void on_pushButtonCall_clicked();

    void on_checkBoxEnable_clicked();

  private:
    Ui::TabPtzAutoHome *ui;
    int m_channel;
    QEventLoop m_eventLoop;
    IpcPtzAutoHome m_ptzAutoHome;
};

#endif // TABPTZAUTOHOME_H
