#include "ActionPageEventPopup.h"
#include "ui_ActionPageEventPopup.h"
#include "ActionAbstract.h"
#include "MyButtonGroup.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MessageBox.h"

ActionPageEventPopup::ActionPageEventPopup(QWidget *parent)
    : ActionPageAbstract(parent)
    , ui(new Ui::ActionPageEventPopup)
{
    ui->setupUi(this);

    m_group = new MyButtonGroup(this);
    m_group->addButton(ui->pushButtonEventPopup, m_action->scheduleType());
    m_group->addButton(ui->pushButtonErase, NONE);
    connect(m_group, SIGNAL(buttonClicked(int)), this, SLOT(onScheduleTypeClicked(int)));

    ui->scheduleWidget->setSingleEditType(m_action->scheduleType());
    if (m_action->eventType() == PEOPLE_COUNT) {
        ui->scheduleWidget->setTypeColor(m_action->scheduleType(), QColor("#FFE793"));
    }

    ui->comboBoxTriggerInterval->clear();
    ui->comboBoxTriggerInterval->addItem("10s", 10);
    ui->comboBoxTriggerInterval->addItem("20s", 20);
    ui->comboBoxTriggerInterval->addItem("40s", 40);
    ui->comboBoxTriggerInterval->addItem("60s", 60);
    ui->comboBoxTriggerInterval->addItem("100s", 100);
    ui->comboBoxTriggerInterval->addItem("5min", 300);
    ui->comboBoxTriggerInterval->addItem("15min", 900);
    ui->comboBoxTriggerInterval->addItem("30min", 1800);
    ui->comboBoxTriggerInterval->addItem("1h", 3600);
    ui->comboBoxTriggerInterval->addItem("8h", 28800);
    ui->comboBoxTriggerInterval->addItem("12h", 43200);
    ui->comboBoxTriggerInterval->addItem("24h", 86400);

    connect(ui->comboBoxNumbersOfChannel, SIGNAL(indexSet(int)), this, SLOT(onComboBoxNumbersOfChannelIndexSet(int)));
    if (m_action->hasTriggerChannelEventPopup()) {
        ui->comboBoxNumbersOfChannel->clear();
        ui->comboBoxNumbersOfChannel->addItem("1", LAYOUTMODE_1);
        ui->comboBoxNumbersOfChannel->addItem("4", LAYOUTMODE_4);
        QList<ComboBox *> comboBoxChannelList;
        comboBoxChannelList.append(ui->comboBoxChannel);
        comboBoxChannelList.append(ui->comboBoxChannel1);
        comboBoxChannelList.append(ui->comboBoxChannel2);
        comboBoxChannelList.append(ui->comboBoxChannel3);
        comboBoxChannelList.append(ui->comboBoxChannel4);
        const int maxChannel = qMsNvr->maxChannel();
        for (int i = 0; i < comboBoxChannelList.size(); ++i) {
            ComboBox *comboBox = comboBoxChannelList.at(i);
            comboBox->clear();
            comboBox->addItem(GET_TEXT("ACTION/56010", "Select Channel"), -1);
            for (int j = 0; j < maxChannel; ++j) {
                comboBox->addItem(QString::number(j + 1), j);
            }
        }
    } else {
        ui->groupBoxTriggerChannelEventPopup->hide();
    }
}

ActionPageEventPopup::~ActionPageEventPopup()
{
    delete ui;
}

void ActionPageEventPopup::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

int ActionPageEventPopup::loadData()
{
    if (hasCache()) {
    } else {
        m_group->setCurrentButton(ui->pushButtonEventPopup);
        ui->scheduleWidget->setSchedule(m_action->eventPopupSchedule());

        ui->comboBoxTriggerInterval->setCurrentIndexFromData(*m_action->eventPopupTriggerInterval());
        ui->labelEventPopup->setColor(m_action->actionColor());

        if (m_action->hasTriggerChannelEventPopup()) {
            int layout = *m_action->eventPopupTriggerLayout();
            int *channels = m_action->eventPopupTriggerChannels();
            switch (layout) {
            case LAYOUTMODE_1:
                break;
            case LAYOUTMODE_4:
                break;
            default:
                //数据库初始化值为0，这里默认为LAYOUTMODE_1
                layout = LAYOUTMODE_1;
            }
            ui->comboBoxNumbersOfChannel->setCurrentIndexFromData(layout);
            ui->comboBoxChannel->setCurrentIndexFromData(channels[0]);
            ui->comboBoxChannel1->setCurrentIndexFromData(channels[0]);
            ui->comboBoxChannel2->setCurrentIndexFromData(channels[1]);
            ui->comboBoxChannel3->setCurrentIndexFromData(channels[2]);
            ui->comboBoxChannel4->setCurrentIndexFromData(channels[3]);
        }

        setCached();
    }
    return 0;
}

int ActionPageEventPopup::saveData()
{
    ui->scheduleWidget->getSchedule(m_action->eventPopupSchedule());
    //
    int *triggerInterval = m_action->eventPopupTriggerInterval();
    *triggerInterval = ui->comboBoxTriggerInterval->currentData().toInt();
    //
    if (m_action->hasTriggerChannelEventPopup()) {
        int *layout = m_action->eventPopupTriggerLayout();
        *layout = ui->comboBoxNumbersOfChannel->currentData().toInt();
        int *channels = m_action->eventPopupTriggerChannels();
        QList<int> channelList = { -1, -1, -1, -1 };
        switch (*layout) {
        case LAYOUTMODE_1: {
            int channel = ui->comboBoxChannel->currentData().toInt();
            if (channel >= 0  && channelList.contains(channel)) {
                MessageBox::information(this, GET_TEXT("ACTION/56011", "One channel can only exist in one window."));
                return -1;
            }
            channelList[0] = ui->comboBoxChannel->currentData().toInt();
            channels[0] = ui->comboBoxChannel->currentData().toInt();
            break;
        }
        case LAYOUTMODE_4: {
            int tempChannel = ui->comboBoxChannel1->currentData().toInt();
            if (tempChannel >= 0  && channelList.contains(tempChannel)) {
                MessageBox::information(this, GET_TEXT("ACTION/56011", "One channel can only exist in one window."));
                return -1;
            }
            channels[0] = tempChannel;
            channelList[0] = tempChannel;

            tempChannel = ui->comboBoxChannel2->currentData().toInt();
            if (tempChannel >= 0  && channelList.contains(tempChannel)) {
                MessageBox::information(this, GET_TEXT("ACTION/56011", "One channel can only exist in one window."));
                return -1;
            }
            channels[1] = tempChannel;
            channelList[1] = tempChannel;

            tempChannel = ui->comboBoxChannel3->currentData().toInt();
            if (tempChannel >= 0  && channelList.contains(tempChannel)) {
                MessageBox::information(this, GET_TEXT("ACTION/56011", "One channel can only exist in one window."));
                return -1;
            }
            channels[2] = ui->comboBoxChannel3->currentData().toInt();
            channelList[2] = tempChannel;

            tempChannel = ui->comboBoxChannel4->currentData().toInt();
            if (tempChannel >= 0  && channelList.contains(tempChannel)) {
                MessageBox::information(this, GET_TEXT("ACTION/56011", "One channel can only exist in one window."));
                return -1;
            }
            channels[3] = ui->comboBoxChannel4->currentData().toInt();
            channelList[3] = tempChannel;
            break;
        }
        }
    }
    return 0;
}

void ActionPageEventPopup::onLanguageChanged()
{
    ui->pushButtonEventPopup->setText(GET_TEXT("SYSTEMGENERAL/70041", "Event Popup"));
    ui->pushButtonErase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
    ui->labelTriggerInterval->setText(GET_TEXT("VIDEOLOSS/50006", "Triggered Interval"));
    ui->groupBoxTriggerChannelEventPopup->setTitle(GET_TEXT("ALARMIN/52012", "Trigger Channel Event Popup"));
    ui->labelNumbersOfChannel->setText(GET_TEXT("ACTION/56006", "Numbers of Channel"));
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelHeaderWindow->setText(GET_TEXT("ACTION/56008", "Window"));
    ui->labelHeaderChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
}

void ActionPageEventPopup::onScheduleTypeClicked(int id)
{
    ui->scheduleWidget->setCurrentType(id);
}

void ActionPageEventPopup::onComboBoxNumbersOfChannelIndexSet(int index)
{
    int layout = ui->comboBoxNumbersOfChannel->itemData(index).toInt();
    switch (layout) {
    case LAYOUTMODE_1:
        ui->labelChannel->show();
        ui->comboBoxChannel->show();
        ui->widgetMultiChannel->hide();
        break;
    case LAYOUTMODE_4:
        ui->labelChannel->hide();
        ui->comboBoxChannel->hide();
        ui->widgetMultiChannel->show();
        break;
    }
}
