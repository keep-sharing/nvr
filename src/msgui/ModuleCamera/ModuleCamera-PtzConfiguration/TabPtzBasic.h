#ifndef TABPTZBASIC_H
#define TABPTZBASIC_H

#include "ptzbasepage.h"
#include <QEventLoop>

extern "C" {
#include "msdb.h"
#include "msg.h"
}

namespace Ui {
class PtzBasicPage;
}

class TabPtzBasic : public PtzBasePage {
    Q_OBJECT

    enum PtzBasicStatus {
        BasicAlwaysClose = 0,
        BasicAlwaysOpen,
        Basic2seconds,
        Basic5seconds,
        Basic10seconds,
    };

public:
    explicit TabPtzBasic(QWidget *parent = 0);
    ~TabPtzBasic();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void setPtzEnable(bool enable);
    void ON_RESPONSE_FLAG_GET_PTZ_BASIC(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_PTZ_BASIC(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_SYSTEM_INFO(MessageReceive *message);

private slots:
    void onLanguageChanged();

    void on_comboBoxChannel_activated(int index);

    void on_pushButtonCopy_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

    void on_comboBoxRecovering_currentIndexChanged(int index);

    void on_comboBoxZoomStatus_currentIndexChanged(int index);
    void on_comboBoxPanStatus_currentIndexChanged(int index);
    void on_comboBoxPresetStatus_currentIndexChanged(int index);
    void on_comboBoxPatrolStatus_currentIndexChanged(int index);
    void on_comboBoxPatternStatus_currentIndexChanged(int index);
    void on_comboBoxAutoStatus_currentIndexChanged(int index);

    void on_comboBoxPreset_currentIndexChanged(int index);

    void on_comboBoxSpeed_currentIndexChanged(int index);
    void on_comboBoxManualSpeed_currentIndexChanged(int index);

    void on_comboBoxFocusMode_currentIndexChanged(int index);
    void on_comboBoxFocusDistance_currentIndexChanged(int index);

    void on_comboBoxResumeTime_currentIndexChanged(int index);

    void on_comboBoxDehumidifying_indexSet(int index);

private:
    Ui::PtzBasicPage *ui;
    int m_channel;

    QEventLoop m_eventLoop;
    PtzBasicInfo m_basic;
    QList<int> m_copyList;
};

#endif // TABPTZBASIC_H
