#include "ActionPageAlarmOutput.h"
#include "ui_ActionPageAlarmOutput.h"
#include "ActionAbstract.h"
#include "AddAlarmOutput.h"
#include "EditAlarmOutput.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"

extern "C" {
#include "msdb.h"
}

const int AlarmOutputRole = Qt::UserRole + 50;

ActionPageAlarmOutput::ActionPageAlarmOutput(QWidget *parent)
    : ActionPageAbstract(parent)
    , ui(new Ui::ActionPageAlarmOutput)
{
    ui->setupUi(this);

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

    QStringList headerAlarmOutput;
    headerAlarmOutput << "";
    headerAlarmOutput << GET_TEXT("ALARMOUT/53001", "Alarm Output No.");
    headerAlarmOutput << GET_TEXT("ALARMIN/52003", "Alarm Name");
    headerAlarmOutput << GET_TEXT("COMMON/1019", "Edit");
    headerAlarmOutput << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableViewAlarmOutput->setHorizontalHeaderLabels(headerAlarmOutput);
    ui->tableViewAlarmOutput->setHeaderCheckable(false);
    ui->tableViewAlarmOutput->setColumnCount(headerAlarmOutput.size());
    ui->tableViewAlarmOutput->setSortingEnabled(false);
    ui->tableViewAlarmOutput->hideColumn(AlarmOutCheck);
    //delegate
    ui->tableViewAlarmOutput->setItemDelegateForColumn(AlarmOutNo, new ItemIconDelegate(this));
    ui->tableViewAlarmOutput->setItemDelegateForColumn(AlarmOutEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableViewAlarmOutput->setItemDelegateForColumn(AlarmOutDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //
    ui->tableViewAlarmOutput->setColumnWidth(AlarmOutNo, 200);
    ui->tableViewAlarmOutput->setColumnWidth(AlarmOutName, 450);
    ui->tableViewAlarmOutput->setColumnWidth(AlarmOutEdit, 100);
    //
    connect(ui->tableViewAlarmOutput, SIGNAL(itemClicked(int, int)), this, SLOT(onTableViewAlarmOutputClicked(int, int)));
    onLanguageChanged();
}

ActionPageAlarmOutput::~ActionPageAlarmOutput()
{
    delete ui;
}

int ActionPageAlarmOutput::loadData()
{
    if (hasCache()) {
    } else {
        //
        ui->comboBoxTriggerInterval->setCurrentIndexFromData(*m_action->alarmOutputTriggerInterval());
        //
        m_alarmoutMap.clear();
        uint *nvrTrigger = m_action->nvrTriggerAlarmOutput();
        alarm_out alarmout_array[MAX_ALARM_OUT];
        memset(alarmout_array, 0, sizeof(alarm_out) * MAX_ALARM_OUT);
        int alarmoutCount = 0;
        read_alarm_outs(SQLITE_FILE_NAME, alarmout_array, &alarmoutCount);
        for (int i = 0; i < MAX_ALARM_OUT && i < qMsNvr->maxAlarmOutput(); ++i) {
            const alarm_out &alarm = alarmout_array[i];
            bool checked = *nvrTrigger & (static_cast<uint>(1) << i);
            AlarmKey key(i, alarm.name);
            m_alarmoutMap.insert(key, checked);
        }

        char *ch1Trigger = m_action->ch1TriggerAlarmOutput();
        alarm_chn_out_name name_array_1[MAX_REAL_CAMERA];
        memset(&name_array_1, 0, sizeof(alarm_chn_out_name) * MAX_REAL_CAMERA);
        read_alarm_chnOut_event_names(SQLITE_FILE_NAME, name_array_1, MAX_REAL_CAMERA, 0);
        for (int i = 0; i < MAX_REAL_CAMERA && i < qMsNvr->maxChannel(); ++i) {
            const alarm_chn_out_name &name = name_array_1[i];
            bool checked = (ch1Trigger[i] == '1');
            AlarmKey key(name.chnid, 0, name.name);
            m_alarmoutMap.insert(key, checked);
        }

        char *ch2Trigger = m_action->ch2TriggerAlarmOutput();
        alarm_chn_out_name name_array_2[MAX_REAL_CAMERA];
        memset(&name_array_2, 0, sizeof(alarm_chn_out_name) * MAX_REAL_CAMERA);
        read_alarm_chnOut_event_names(SQLITE_FILE_NAME, name_array_2, MAX_REAL_CAMERA, 1);
        for (int i = 0; i < MAX_REAL_CAMERA && i < qMsNvr->maxChannel(); ++i) {
            const alarm_chn_out_name &name = name_array_2[i];
            bool checked = (ch2Trigger[i] == '1');
            AlarmKey key(name.chnid, 1, name.name);
            m_alarmoutMap.insert(key, checked);
        }

        updateTriggerAlarmOutputTable();

        //
        setCached();
    }
    return 0;
}

int ActionPageAlarmOutput::saveData()
{
    //
    int *triggerInterval = m_action->alarmOutputTriggerInterval();
    *triggerInterval = ui->comboBoxTriggerInterval->currentData().toInt();
    //
    uint *nvrTrigger = m_action->nvrTriggerAlarmOutput();
    uint tri_alarms = 0;
    int index = 0;
    for (auto iter = m_alarmoutMap.constBegin(); iter != m_alarmoutMap.constEnd(); ++iter) {
        const AlarmKey &key = iter.key();
        bool checked = iter.value();
        if (key.type() == AlarmKey::NvrAlarm) {
            if (checked) {
                tri_alarms |= (1 << index);
            } else {
                tri_alarms &= ~(1 << index);
            }
            index++;
        }
    }
    *nvrTrigger = tri_alarms;
    //
    char *ch1Trigger = m_action->ch1TriggerAlarmOutput();
    char *ch2Trigger = m_action->ch2TriggerAlarmOutput();
    char tri_alarms1[MAX_LEN_65];
    char tri_alarms2[MAX_LEN_65];
    memset(tri_alarms1, 0, sizeof(tri_alarms1));
    memset(tri_alarms2, 0, sizeof(tri_alarms2));
    for (auto iter = m_alarmoutMap.constBegin(); iter != m_alarmoutMap.constEnd(); ++iter) {
        const AlarmKey &key = iter.key();
        bool checked = iter.value();
        if (key.type() == AlarmKey::CameraAlarm) {
            if (key.alarmId() == 0) {
                if (checked) {
                    tri_alarms1[key.channel()] = '1';
                } else {
                    tri_alarms1[key.channel()] = '0';
                }
            } else if (key.alarmId() == 1) {
                if (checked) {
                    tri_alarms2[key.channel()] = '1';
                } else {
                    tri_alarms2[key.channel()] = '0';
                }
            }
        }
    }
    memcpy(ch1Trigger, tri_alarms1, MAX_LEN_65);
    memcpy(ch2Trigger, tri_alarms2, MAX_LEN_65);
    return 0;
}

void ActionPageAlarmOutput::updateTriggerAlarmOutputTable()
{
    ui->tableViewAlarmOutput->clearContent();
    int row = 0;
    for (auto iter = m_alarmoutMap.constBegin(); iter != m_alarmoutMap.constEnd(); ++iter) {
        const AlarmKey &key = iter.key();
        bool value = iter.value();
        if (value) {
            ui->tableViewAlarmOutput->setItemText(row, AlarmOutNo, key.numberName());
            ui->tableViewAlarmOutput->setItemData(row, AlarmOutNo, QVariant::fromValue(key), AlarmOutputRole);
            ui->tableViewAlarmOutput->setItemText(row, AlarmOutName, key.name());
            row++;
        }
    }
    static QPixmap pixmap(":/ptz/ptz/add.png");
    ui->tableViewAlarmOutput->setItemPixmap(row, AlarmOutNo, pixmap);
    ui->tableViewAlarmOutput->setItemText(row, AlarmOutName, "-");
    ui->tableViewAlarmOutput->setItemText(row, AlarmOutEdit, "-");
    ui->tableViewAlarmOutput->setItemText(row, AlarmOutDelete, "-");
}

void ActionPageAlarmOutput::onLanguageChanged()
{
    ui->labelTriggerAlarmOutput->setText(GET_TEXT("VIDEOLOSS/50008", "Trigger Alarm Output"));
    ui->labelTriggerInterval->setText(GET_TEXT("VIDEOLOSS/50006", "Triggered Interval"));
}

void ActionPageAlarmOutput::onTableViewAlarmOutputClicked(int row, int column)
{
    int rowCount = 0;
    for (auto iter = m_alarmoutMap.constBegin(); iter != m_alarmoutMap.constEnd(); ++iter) {
        bool checked = iter.value();
        if (checked) {
            rowCount++;
        }
    }

    switch (column) {
    case AlarmOutNo:
        if (row == rowCount) {
            AddAlarmOutput add(&m_alarmoutMap, this);
            int result = add.exec();
            if (result == AddAlarmOutput::Accepted) {
                updateTriggerAlarmOutputTable();
            }
        }
        break;
    case AlarmOutName:
        break;
    case AlarmOutEdit: {
        if (row < rowCount) {
            const AlarmKey &key = ui->tableViewAlarmOutput->itemData(row, AlarmOutNo, AlarmOutputRole).value<AlarmKey>();
            EditAlarmOutput edit(&m_alarmoutMap, this);
            int result = edit.execEdit(key);
            if (result == AddAlarmOutput::Accepted) {
                updateTriggerAlarmOutputTable();
            }
        }
        break;
    }
    case AlarmOutDelete:
        if (row < rowCount) {
            const AlarmKey &key = ui->tableViewAlarmOutput->itemData(row, AlarmOutNo, AlarmOutputRole).value<AlarmKey>();
            m_alarmoutMap[key] = false;
            updateTriggerAlarmOutputTable();
        }
        break;
    }
}
