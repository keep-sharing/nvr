#include "ActionPageEmailLinkage.h"
#include "ui_ActionPageEmailLinkage.h"
#include "ActionAbstract.h"
#include "MyButtonGroup.h"
#include "channelcheckbox.h"
#include "MsDevice.h"
#include "MsLanguage.h"

ActionPageEmailLinkage::ActionPageEmailLinkage(QWidget *parent)
    : ActionPageAbstract(parent)
    , ui(new Ui::ActionPageEmailLinkage)
{
    ui->setupUi(this);

    m_group = new MyButtonGroup(this);
    m_group->addButton(ui->pushButtonEmail, m_action->scheduleType());
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

    ui->comboBoxPictureAttached->clear();
    ui->comboBoxPictureAttached->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxPictureAttached->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    if (m_action->hasTriggerChannelsSnapshot()) {
        int maxChannel = qMsNvr->maxChannel();
        for (int i = 0; i < maxChannel; i++) {
            ChannelCheckBox *checkBox = new ChannelCheckBox(this);
            checkBox->setStyleSheet("min-height: 20px;");
            checkBox->setChannel(i);
            checkBox->setText(QString("%1").arg(i + 1));
            ui->gridLayout_snapshot->addWidget(checkBox, i / 8, i % 8);
            connect(checkBox, SIGNAL(clicked(int, bool)), this, SLOT(onSnapshotCheckBoxChecked(int, bool)));
            m_checkBoxList.append(checkBox);
        }
    } else {
        ui->groupBoxSnapshot->hide();
    }
}

ActionPageEmailLinkage::~ActionPageEmailLinkage()
{
    delete ui;
}

void ActionPageEmailLinkage::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

int ActionPageEmailLinkage::loadData()
{
    if (hasCache()) {
    } else {
        m_group->setCurrentButton(ui->pushButtonEmail);
        ui->scheduleWidget->setSchedule(m_action->emailLinkageSchedule());

        ui->comboBoxTriggerInterval->setCurrentIndexFromData(*m_action->emailLinkageTriggerInterval());
        if (m_action->emailLinkagePictureAttached() != nullptr) {
            ui->comboBoxPictureAttached->setCurrentIndexFromData(*m_action->emailLinkagePictureAttached());
        } else {
            ui->comboBoxPictureAttached->hide();
            ui->labelPictureAttached->hide();
        }
        ui->labelEmail->setColor(m_action->actionColor());

        if (m_action->hasTriggerChannelsSnapshot()) {
            char *triChannels = m_action->emailLinkageTriggerChannelsSnapshot();
            int maxChannel = qMsNvr->maxChannel();
            for (int i = 0; i < m_checkBoxList.size() && i < maxChannel; i++) {
                ChannelCheckBox *checkBox = m_checkBoxList.at(i);
                if (triChannels[i] == '1') {
                    checkBox->setChecked(true);
                } else {
                    checkBox->setChecked(false);
                }
            }
        }

        setCached();
    }
    return 0;
}

int ActionPageEmailLinkage::saveData()
{
    ui->scheduleWidget->getSchedule(m_action->emailLinkageSchedule());
    //
    int *triggerInterval = m_action->emailLinkageTriggerInterval();
    *triggerInterval = ui->comboBoxTriggerInterval->currentData().toInt();
    //
    if (m_action->emailLinkagePictureAttached() != nullptr) {
        int *pictureAttached = m_action->emailLinkagePictureAttached();
        *pictureAttached = ui->comboBoxPictureAttached->currentData().toInt();
    }
    //
    if (m_action->hasTriggerChannelsSnapshot()) {
        char *triChannels = m_action->emailLinkageTriggerChannelsSnapshot();
        int size = m_action->emailLinkageTriggerChannelsSnapshotArraySize();
        memset(triChannels, 0, size);
        int maxChannel = qMsNvr->maxChannel();
        for (int i = 0; i < m_checkBoxList.size() && i < maxChannel; i++) {
            ChannelCheckBox *checkBox = m_checkBoxList.at(i);
            if (checkBox->isChecked()) {
                triChannels[i] = '1';
            } else {
                triChannels[i] = '0';
            }
        }
    }
    return 0;
}

void ActionPageEmailLinkage::onLanguageChanged()
{
    ui->pushButtonEmail->setText(GET_TEXT("VIDEOLOSS/50005", "Email"));
    ui->pushButtonErase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
    ui->labelTriggerInterval->setText(GET_TEXT("VIDEOLOSS/50006", "Triggered Interval"));
    ui->labelPictureAttached->setText(GET_TEXT("ACTION/56003", "Picture Attached"));
}

void ActionPageEmailLinkage::onScheduleTypeClicked(int id)
{
    ui->scheduleWidget->setCurrentType(id);
}

void ActionPageEmailLinkage::onSnapshotCheckBoxChecked(int channel, bool checked)
{
    Q_UNUSED(channel)
    Q_UNUSED(checked)

    int checkedCount = 0;
    for (int i = 0; i < m_checkBoxList.size(); i++) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            checkedCount++;
        }
    }

    if (checkedCount >= 16) {
        for (int i = 0; i < m_checkBoxList.size(); i++) {
            ChannelCheckBox *checkBox = m_checkBoxList.at(i);
            if (!checkBox->isChecked()) {
                checkBox->setEnabled(false);
            }
        }
    } else {
        for (int i = 0; i < m_checkBoxList.size(); i++) {
            ChannelCheckBox *checkBox = m_checkBoxList.at(i);
            checkBox->setEnabled(true);
        }
    }
}
