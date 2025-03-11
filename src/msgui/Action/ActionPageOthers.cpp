#include "ActionPageOthers.h"
#include "ui_ActionPageOthers.h"
#include "ActionAbstract.h"
#include "MsDevice.h"
#include "MsLanguage.h"

ActionPageOthers::ActionPageOthers(QWidget *parent)
    : ActionPageAbstract(parent)
    , ui(new Ui::ActionPageOthers)
{
    ui->setupUi(this);

    if (!qMsNvr->isSupportEventSnapshot()) {
        ui->labelTriggerChannelsSnapshot->hide();
        ui->checkBoxGroupSnapshot->hide();
    }

    ui->labelNote->setVisible(m_action->isNoteVisible());
}

ActionPageOthers::~ActionPageOthers()
{
    delete ui;
}

void ActionPageOthers::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

int ActionPageOthers::loadData()
{
    if (hasCache()) {
    } else {
        ui->checkBoxGroupRecord->setCount(qMsNvr->maxChannel());
        ui->checkBoxGroupRecord->setCheckedFromString(QString(m_action->triggerChannelsRecordEx()));
        if (qMsNvr->isSupportEventSnapshot()) {
            ui->checkBoxGroupSnapshot->setCount(qMsNvr->maxChannel());
            ui->checkBoxGroupSnapshot->setCheckedFromString(QString(m_action->triggerChannelsSnapshot()));
        }
        //
        setCached();
    }
    return 0;
}

int ActionPageOthers::saveData()
{
    uint *tri_channels = m_action->triggerChannelsRecord();
    char *tri_channels_ex = m_action->triggerChannelsRecordEx();
    QList<bool> otherList = ui->checkBoxGroupRecord->checkStateList();
    for (int i = 0; i < otherList.size(); ++i) {
        if (otherList.at(i)) {
            *tri_channels |= (1 << i);
            tri_channels_ex[i] = '1';
        } else {
            *tri_channels &= ~(1 << i);
            tri_channels_ex[i] = '0';
        }
    }

    char *tri_channels_pic = m_action->triggerChannelsSnapshot();
    QList<bool> snapshotList = ui->checkBoxGroupSnapshot->checkStateList();
    for (int i = 0; i < snapshotList.size(); ++i) {
        if (snapshotList.at(i)) {
            tri_channels_pic[i] = '1';
        } else {
            tri_channels_pic[i] = '0';
        }
    }
    return 0;
}

void ActionPageOthers::onLanguageChanged()
{
    ui->labelTriggerChannelsRecord->setText(GET_TEXT("VIDEOLOSS/50007", "Trigger Channels Record"));
    ui->labelTriggerChannelsSnapshot->setText(GET_TEXT("ALARMIN/52011", "Trigger Channels Snapshot"));
    ui->labelNote->setText(GET_TEXT("VIDEOLOSS/50022", "Note: This page's configuration will not be copied into other channels."));
}
