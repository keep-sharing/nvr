#ifndef TABPTZLIMIT_H
#define TABPTZLIMIT_H

#include "QEventLoop"
#include "ptzbasepage.h"

extern "C" {
#include "msg.h"
}

namespace Ui {
class TabPtzLimit;
}

class TabPtzLimit : public PtzBasePage {
    Q_OBJECT

public:
    explicit TabPtzLimit(QWidget *parent = nullptr);
    ~TabPtzLimit() override;

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPC_PTZ_LIMIT(MessageReceive *message);
    void setSettingEnable(bool enable);
    void updateMyInfo(PTZ_LIMITS_TYPE type, int value);
    void changeMask(int &order, int bit, int value);

private slots:
    void setLimitControl(PTZ_LIMITS_TYPE type, int value);
    void onLanguageChanged();
    void onChannelGroupClicked(int channel);
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

    void on_comboBoxLimitMode_indexSet(int index);

    void on_checkBoxEnable_clicked();

  private:
    Ui::TabPtzLimit *ui;
    int m_channel;
    QEventLoop m_eventLoop;
    IpcPtzLimitINFO m_limitInfo;
};

#endif // TABPTZLIMIT_H
