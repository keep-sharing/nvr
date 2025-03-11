#include "ActionPageAudibleWarning.h"
#include "ui_ActionPageAudibleWarning.h"
#include "ActionAbstract.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyButtonGroup.h"

ActionPageAudibleWarning::ActionPageAudibleWarning(QWidget *parent)
    : ActionPageAbstract(parent)
    , ui(new Ui::ActionPageAudibleWarning)
{
    ui->setupUi(this);

    m_group = new MyButtonGroup(this);
    m_group->addButton(ui->pushButtonAudible, m_action->scheduleType());
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

    ui->comboBoxAudioFile->beginEdit();
    ui->comboBoxAudioFile->clear();
    ui->comboBoxAudioFile->addItem(GET_TEXT("COMMON/1050", "Default"), 0);
    ui->comboBoxAudioFile->addItem("1", 1);
    ui->comboBoxAudioFile->addItem("2", 2);
    ui->comboBoxAudioFile->addItem("3", 3);
    ui->comboBoxAudioFile->addItem("4", 4);
    ui->comboBoxAudioFile->addItem("5", 5);
    ui->comboBoxAudioFile->addItem("6", 6);
    ui->comboBoxAudioFile->addItem("7", 7);
    ui->comboBoxAudioFile->addItem("8", 8);
    ui->comboBoxAudioFile->addItem("9", 9);
    ui->comboBoxAudioFile->addItem("10", 10);
    ui->comboBoxAudioFile->endEdit();

    if  (!qMsNvr->isSupportAudio()) {
        ui->labelAudioFile->hide();
        ui->comboBoxAudioFile->hide();
    }

}

ActionPageAudibleWarning::~ActionPageAudibleWarning()
{
    delete ui;
}

void ActionPageAudibleWarning::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

int ActionPageAudibleWarning::loadData()
{
    if (hasCache()) {
    } else {
        //
        m_group->setCurrentButton(ui->pushButtonAudible);
        ui->scheduleWidget->setSchedule(m_action->audibleWarningSchedule());
        //
        ui->comboBoxTriggerInterval->setCurrentIndexFromData(*m_action->audibleWarningTriggerInterval());
        ui->labelAudible->setColor(m_action->actionColor());
        //
        ui->comboBoxAudioFile->setCurrentIndexFromData(*m_action->audioFileNo());
        setCached();
    }
    return 0;
}

int ActionPageAudibleWarning::saveData()
{
    ui->scheduleWidget->getSchedule(m_action->audibleWarningSchedule());
    //
    int *triggerInterval = m_action->audibleWarningTriggerInterval();
    *triggerInterval = ui->comboBoxTriggerInterval->currentData().toInt();

    int *audioFileNo = m_action->audioFileNo();
    *audioFileNo = ui->comboBoxAudioFile->currentData().toInt();
    return 0;
}

void ActionPageAudibleWarning::onLanguageChanged()
{
    ui->pushButtonAudible->setText(GET_TEXT("VIDEOLOSS/50004", "Audible"));
    ui->pushButtonErase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
    ui->labelAudioFile->setText(GET_TEXT("AUDIOFILE/117012", "Audio File"));
    ui->labelTriggerInterval->setText(GET_TEXT("VIDEOLOSS/50006", "Triggered Interval"));
}

void ActionPageAudibleWarning::onScheduleTypeClicked(int id)
{
    ui->scheduleWidget->setCurrentType(id);
}
