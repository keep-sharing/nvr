#include "PtzScheduleTask.h"
#include "ui_PtzScheduleTask.h"
#include "MsLanguage.h"

PtzScheduleTask::PtzScheduleTask(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::PtzScheduleTask)
{
    ui->setupUi(this);
    m_schedultEdit = new PtzScheduleTaskEdit(this);
    connect(m_schedultEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    QButtonGroup *buttonGroup = new QButtonGroup(this);

    buttonGroup->addButton(ui->pushButtonClose, 0);
    buttonGroup->addButton(ui->pushButtonAutoScan, 1);
    buttonGroup->addButton(ui->pushButtonPreset, 2);
    buttonGroup->addButton(ui->pushButtonPatrol, 3);
    buttonGroup->addButton(ui->pushButtonPattern, 4);
    buttonGroup->addButton(ui->pushButtonSelfCheck, 5);
    buttonGroup->addButton(ui->pushButtonTiltScan, 6);
    buttonGroup->addButton(ui->pushButtonPanoramaScan, 7);
    buttonGroup->addButton(ui->pushButtonErase, 8);
    buttonGroup->addButton(ui->pushButtonErase2, 9);
    connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(onButtonGroupClicked(QAbstractButton *)));
    connect(ui->pushButtonPreset, SIGNAL(itemClicked()), this, SLOT(on_pushButtonPreset_clicked()));
    connect(ui->pushButtonPatrol, SIGNAL(itemClicked()), this, SLOT(on_pushButtonPatrol_clicked()));
    connect(ui->pushButtonPattern, SIGNAL(itemClicked()), this, SLOT(on_pushButtonPattern_clicked()));

    ui->scheduleDraw->setScheduleMode(MsScheduleDraw::Mode_NoHoliday);
    ui->scheduleDraw->setTypeColor(ACTION_ERASE, QColor("#DEDEDE"));
    ui->scheduleDraw->setTypeColor(ACTION_CLOSE, QColor("#333333"));
    ui->scheduleDraw->setTypeColor(ACTION_AUTOSCAN, QColor("#95A5E8"));
    ui->scheduleDraw->setTypeColor(ACTION_PRESET, QColor("#A0C78E"));
    ui->scheduleDraw->setTypeColor(ACTION_PATROL, QColor("#E3AC5D"));
    ui->scheduleDraw->setTypeColor(ACTION_PATTERN, QColor("#C9E2F2"));
    ui->scheduleDraw->setTypeColor(ACTION_CHECK, QColor("#EE9B9E"));
    ui->scheduleDraw->setTypeColor(ACTION_TILTSCAN, QColor("#A571D5"));
    ui->scheduleDraw->setTypeColor(ACTION_PANORAMASCAN, QColor("#D55AD5"));

    for (int i = 101; i <= 400; i++) {
        ui->scheduleDraw->setTypeColor(i, QColor("#A0C78E"));
    }
    for (int i = 401; i <= 408; i++) {
        ui->scheduleDraw->setTypeColor(i, QColor("#E3AC5D"));
    }
    for (int i = 501; i <= 504; i++) {
        ui->scheduleDraw->setTypeColor(i, QColor("#C9E2F2"));
    }

    onLanguageChanged();
}

PtzScheduleTask::~PtzScheduleTask()
{
    delete ui;
}

void PtzScheduleTask::showAction(IPC_PTZ_SCHE_TASK_WEEK_S &schedule, int supportWiper, int supportSpeed)
{
    setSchedule(schedule);
    m_supportWiper = supportWiper;
    m_supportSpeed = supportSpeed;
    int limit = supportWiper ? 53 : 52;
    ui->pushButtonPreset->clear();
    for (int i = 1; i <= 300; i++) {
        if (i >= 33 && i <= limit) {
            continue;
        }
        ui->pushButtonPreset->addItem(GET_TEXT("PTZDIALOG/21015", "Preset") + QString("%1").arg(i), i);
    }
    ui->pushButtonPreset->setCurrentIndex(0);

    ui->pushButtonPatrol->clear();
    for (int i = 1; i <= 8; i++) {
        ui->pushButtonPatrol->addItem(GET_TEXT("PTZDIALOG/21016", "Patrol") + QString("%1").arg(i), i);
    }
    ui->pushButtonPatrol->setCurrentIndex(0);

    ui->pushButtonPattern->clear();
    for (int i = 1; i <= 4; i++) {
        ui->pushButtonPattern->addItem(GET_TEXT("PTZDIALOG/21017", "Pattern") + QString("%1").arg(i), i);
    }
    ui->pushButtonPattern->setCurrentIndex(0);

    ui->widgetErase->setVisible(supportSpeed);
    ui->widgetTiltScan->setVisible(supportSpeed);
    ui->widgetPanoramaScan->setVisible(supportSpeed);
    ui->widgetErase2->setVisible(!supportSpeed);
    ui->widgetEmpty->setVisible(!supportSpeed);

    if (supportSpeed) {
        ui->pushButtonErase->setChecked(true);
        on_pushButtonErase_clicked();
    } else {
        ui->pushButtonErase2->setChecked(true);
        on_pushButtonErase2_clicked();
    }
}

void PtzScheduleTask::setSchedule(IPC_PTZ_SCHE_TASK_WEEK_S &schedule)
{
    memset(m_schedule_day, 0, sizeof(schedule_day) * MAX_DAY_NUM);
    for (int day = 0; day < MAX_DAY_NUM_IPC; day++) {
        uint16_t secArray[86401] { 0 };
        for (int item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            if (!schedule.scheDay[day].scheItem[item].isValid) {
                break;
            }
            int start = (schedule.scheDay[day].scheItem[item].startHour * 3600) + (schedule.scheDay[day].scheItem[item].startMin * 60);
            int end = (schedule.scheDay[day].scheItem[item].endHour * 3600) + (schedule.scheDay[day].scheItem[item].endMin * 60);
            for (int x = start; x <= end; x++) {
                if (schedule.scheDay[day].scheItem[item].type == static_cast<PTZ_SCHE_TYPE>(ACTION_PRESET)) {
                    secArray[x] = static_cast<uint16_t>(schedule.scheDay[day].scheItem[item].templateId + 100);
                } else if (schedule.scheDay[day].scheItem[item].type == static_cast<PTZ_SCHE_TYPE>(ACTION_PATROL)) {
                    secArray[x] = static_cast<uint16_t>(schedule.scheDay[day].scheItem[item].templateId + 400);
                } else if (schedule.scheDay[day].scheItem[item].type == static_cast<PTZ_SCHE_TYPE>(ACTION_PATTERN)) {
                    secArray[x] = static_cast<uint16_t>(schedule.scheDay[day].scheItem[item].templateId + 500);
                } else {
                    secArray[x] = static_cast<uint16_t>(schedule.scheDay[day].scheItem[item].type);
                }
            }
        }
        schdule_item *item_array = m_schedule_day[day].schedule_item;
        QList<ScheduleTime> times = ScheduleTime::fromSecondsHash(secArray);
        int index = 0;
        for (int item = 0; item < times.size(); ++item) {
            const ScheduleTime &time = times.at(item);
            schdule_item &s_item = item_array[index];
            s_item.action_type = time.type();
            QString strBegin = QString("%1:%2").arg(time.beginMinute() / 60, 2, 10, QLatin1Char('0')).arg(time.beginMinute() % 60, 2, 10, QLatin1Char('0'));
            QString strEnd = QString("%1:%2").arg(time.endMinute() / 60, 2, 10, QLatin1Char('0')).arg(time.endMinute() % 60, 2, 10, QLatin1Char('0'));
            snprintf(s_item.start_time, sizeof(s_item.start_time), "%s", strBegin.toStdString().c_str());
            snprintf(s_item.end_time, sizeof(s_item.end_time), "%s", strEnd.toStdString().c_str());

            index++;
        }
    }

    ui->scheduleDraw->setSchedule(m_schedule_day);
}

void PtzScheduleTask::getSchedule(IPC_PTZ_SCHE_TASK_WEEK_S &schedule)
{
    memset(&schedule, 0, sizeof(IPC_PTZ_SCHE_TASK_WEEK_S));
    ui->scheduleDraw->getSchedule(m_schedule_day);
    for (int day = 0; day < MAX_DAY_NUM_IPC; day++) {
        QList<ScheduleTime> times = ScheduleTime::fromSecondsHash(ui->scheduleDraw->scheduleArray()[day]);
        int item = 0;
        for (auto scheTime = times.constBegin(); scheTime != times.constEnd(); scheTime++) {
            if (scheTime->type() != 0) {
                schedule.scheDay[day].scheItem[item].startHour = scheTime->beginMinute() / 60;
                schedule.scheDay[day].scheItem[item].startMin = scheTime->beginMinute() % 60;
                schedule.scheDay[day].scheItem[item].endHour = scheTime->endMinute() / 60;
                schedule.scheDay[day].scheItem[item].endMin = scheTime->endMinute() % 60;
                schedule.scheDay[day].scheItem[item].isValid = MS_TRUE;
                if (scheTime->type() < 100) {
                    schedule.scheDay[day].scheItem[item].type = static_cast<PTZ_SCHE_TYPE>(scheTime->type());
                    schedule.scheDay[day].scheItem[item].templateId = 0;
                } else if (scheTime->type() <= 400) {
                    schedule.scheDay[day].scheItem[item].type = static_cast<PTZ_SCHE_TYPE>(ACTION_PRESET);
                    schedule.scheDay[day].scheItem[item].templateId = scheTime->type() - 100;
                } else if (scheTime->type() <= 500) {
                    schedule.scheDay[day].scheItem[item].type = static_cast<PTZ_SCHE_TYPE>(ACTION_PATROL);
                    schedule.scheDay[day].scheItem[item].templateId = scheTime->type() - 400;
                } else {
                    schedule.scheDay[day].scheItem[item].type = static_cast<PTZ_SCHE_TYPE>(ACTION_PATTERN);
                    schedule.scheDay[day].scheItem[item].templateId = scheTime->type() - 500;
                }
                item++;
            }
        }
    }
}

void PtzScheduleTask::onEditingFinished()
{
    ui->scheduleDraw->setScheduleArray(m_schedultEdit->scheduleArray());
}

void PtzScheduleTask::onLanguageChanged()
{
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    ui->pushButtonDefault->setText(GET_TEXT("COMMON/1050", "Default"));
    ui->pushButtonSelectAll->setText(GET_TEXT("COMMON/1022", "Select All"));
    ui->pushButtonEditTime->setText(GET_TEXT("RECORDMODE/90010", "Edit Time"));

    ui->pushButtonClose->setText(GET_TEXT("PTZDIALOG/21005", "Close"));
    ui->pushButtonAutoScan->setText(GET_TEXT("PTZDIALOG/21014", "Auto Scan"));
    ui->pushButtonSelfCheck->setText(GET_TEXT("PTZCONFIG/166012", "Self Check"));
    ui->pushButtonTiltScan->setText(GET_TEXT("PTZCONFIG/166013", "Tilt Scan"));
    ui->pushButtonPanoramaScan->setText(GET_TEXT("PTZCONFIG/166014", "Panorama Scan"));
    ui->pushButtonErase2->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
    ui->pushButtonErase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
}

void PtzScheduleTask::onButtonGroupClicked(QAbstractButton *button)
{
    Q_UNUSED(button)
    ui->pushButtonPreset->clearHover();
    ui->pushButtonPatrol->clearHover();
    ui->pushButtonPattern->clearHover();
}

void PtzScheduleTask::on_pushButtonOk_clicked()
{
    accept();
}

void PtzScheduleTask::on_pushButtonCancel_clicked()
{
    reject();
}

void PtzScheduleTask::on_pushButtonClose_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_CLOSE);
}

void PtzScheduleTask::on_pushButtonAutoScan_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_AUTOSCAN);
}

void PtzScheduleTask::on_pushButtonPreset_clicked()
{
    int id = ui->pushButtonPreset->currentInt() + 100;
    ui->scheduleDraw->setCurrentType(id);
}

void PtzScheduleTask::on_pushButtonPatrol_clicked()
{
    int id = ui->pushButtonPatrol->currentInt() + 400;
    ui->scheduleDraw->setCurrentType(id);
}

void PtzScheduleTask::on_pushButtonPattern_clicked()
{
    int id = ui->pushButtonPattern->currentInt() + 500;
    ui->scheduleDraw->setCurrentType(id);
}

void PtzScheduleTask::on_pushButtonSelfCheck_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_CHECK);
}

void PtzScheduleTask::on_pushButtonTiltScan_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_TILTSCAN);
}

void PtzScheduleTask::on_pushButtonPanoramaScan_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_PANORAMASCAN);
}

void PtzScheduleTask::on_pushButtonErase2_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_ERASE);
}

void PtzScheduleTask::on_pushButtonErase_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_ERASE);
}

void PtzScheduleTask::on_pushButtonDefault_clicked()
{
    ui->pushButtonErase->setChecked(true);
    on_pushButtonErase_clicked();
    ui->scheduleDraw->selectAll();
}

void PtzScheduleTask::on_pushButtonSelectAll_clicked()
{
    ui->scheduleDraw->selectAll();
}

void PtzScheduleTask::on_pushButtonEditTime_clicked()
{
    m_schedultEdit->setScheduleArray(ui->scheduleDraw->scheduleArray(), m_supportWiper, m_supportSpeed);
    m_schedultEdit->exec();
}
