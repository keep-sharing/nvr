#include "exposureschedule.h"
#include "ui_exposureschedule.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QButtonGroup>
#include <QtDebug>

extern "C" {
#include "msdb.h"
#include "msg.h"
}

ExposureSchedule::ExposureSchedule(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::ExposureSchedule)
{
    ui->setupUi(this);
    setTitleWidget(ui->label_title);

    m_schedultEdit = new ExposureScheduleEdit(this);
    connect(m_schedultEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    QButtonGroup *buttonGroup = new QButtonGroup(this);

    buttonGroup->addButton(ui->pushButton_manualMod, ExposureActionAuto);
    buttonGroup->addButton(ui->pushButton_autoMode, ExposureActionManual);

    onLanguageChanged();
}

ExposureSchedule::~ExposureSchedule()
{
  if (m_exposureShce) {
    delete m_exposureShce;
    m_exposureShce = nullptr;
  }
  if (m_exposureShceCache) {
    delete m_exposureShceCache;
    m_exposureShceCache = nullptr;
  }
    delete ui;
}

void ExposureSchedule::showAction(int channel)
{
    m_channel = channel;
    if (!m_isModeSetting) {
        initializeManualMode();
    }
    memset(m_exposureShceCache, 0, sizeof(struct exposure_schedule));
    memcpy(m_exposureShceCache, m_exposureShce, sizeof(struct exposure_schedule));
    m_isModeSetting = true;

    ui->pushButton_autoMode->setChecked(true);
    on_pushButton_autoMode_clicked();
}

void ExposureSchedule::ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE(MessageReceive *message)
{
    struct exposure_schedule *data = (struct exposure_schedule *)message->data;
    if (!data) {
        qWarning() << "ExposureSchedule::ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE, data is null.";
        return;
    }
    if (!m_exposureShce) {
        m_exposureShce = new exposure_schedule;
    }
    if (!m_exposureShceCache) {
        m_exposureShceCache = new exposure_schedule;
    }
    memset(m_exposureShce, 0, sizeof(struct exposure_schedule));
    memcpy(m_exposureShce, data, sizeof(struct exposure_schedule));
    setSchedule(m_exposureShce->schedule_day);
}

void ExposureSchedule::initializeManualMode()
{
    m_mapAllManualMode.clear();
    //先遍历出来ipc返回的ExposureManualValue
    QList<ExposureManualValue> manualModeList;
    for (int day = 0; day < MAX_DAY_NUM; ++day) {
        const exposure_schedule_day &schedule_day = m_exposureShce->schedule_day[day];
        for (int i = 0; i < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; ++i) {
            const exposure_schedule_item &schedule_item = schedule_day.schedule_item[i];
            ExposureManualValue manualValue(schedule_item.action_type, schedule_item.exposureTime, schedule_item.gainLevel);
            if (manualValue.isValid() && !manualModeList.contains(manualValue)) {
                manualModeList.append(manualValue);

            }
        }
    }
    qDebug() << QString("ExposureSchedule::initializeManualMode, exposure_schedule, mode size: %1").arg(manualModeList.size());

    //
    ui->comboBox_manualModeEvent->clear();
    int modeIndex = 0;
    for (auto iter = manualModeList.constBegin(); iter != manualModeList.constEnd(); ++iter) {
        const ExposureManualValue &value = *iter;
        m_mapAllManualMode.insert(modeIndex, value);
        ui->comboBox_manualModeEvent->addItem(QString::number(modeIndex + 1), modeIndex);
        modeIndex++;
    }
    if (!manualModeList.isEmpty()) {
        on_comboBox_manualModeEvent_activated(ui->comboBox_manualModeEvent->currentIndex());
    }
}

void ExposureSchedule::clearCache()
{
    if (m_exposureShce) {
        delete m_exposureShce;
        m_exposureShce = nullptr;
    }
    if (m_exposureShceCache) {
        delete m_exposureShceCache;
        m_exposureShceCache = nullptr;
    }
    m_isModeSetting = false;
}

void ExposureSchedule::resetCache()
{
    memset(m_exposureShce, 0, sizeof(struct exposure_schedule));
    memcpy(m_exposureShce, m_exposureShceCache, sizeof(struct exposure_schedule));
    setSchedule(m_exposureShce->schedule_day);
}

void ExposureSchedule::setManualMode(QList<ExposureManualValue> manualModeList)
{
    m_mapAllManualMode.clear();

    //
    ui->comboBox_manualModeEvent->clear();
    int modeIndex = 0;
    for (auto iter = manualModeList.constBegin(); iter != manualModeList.constEnd(); ++iter) {
        const ExposureManualValue &value = *iter;
        m_mapAllManualMode.insert(modeIndex, value);
        ui->comboBox_manualModeEvent->addItem(QString::number(modeIndex + 1), modeIndex);
        modeIndex++;
    }
    if (!manualModeList.isEmpty()) {
        on_comboBox_manualModeEvent_activated(ui->comboBox_manualModeEvent->currentIndex());
    }
    m_isModeSetting = true;
}

void ExposureSchedule::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("IMAGE/37214", "Exposure Schedule"));
    ui->pushButton_default->setText(GET_TEXT("COMMON/1050", "Default"));
    ui->pushButton_selectAll->setText(GET_TEXT("COMMON/1022", "Select All"));
    ui->pushButton_editTime->setText(GET_TEXT("RECORDMODE/90010", "Edit Time"));
    ui->pushButton_manualMod->setText(GET_TEXT("IMAGE/37203", "Manual Mode"));
    ui->pushButton_autoMode->setText(GET_TEXT("IMAGE/37104", "Auto Mode"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void ExposureSchedule::setSchedule(exposure_schedule_day *schedule_day_array)
{
    ui->scheduleDraw->setSchedule(schedule_day_array);
}

void ExposureSchedule::getSchedule(exposure_schedule_day *schedule_day_array)
{
    ui->scheduleDraw->getSchedule(schedule_day_array);
}

void ExposureSchedule::onEditingFinished()
{
    ui->scheduleDraw->setScheduleArray(m_schedultEdit->scheduleArray());
}

void ExposureSchedule::onButtonGroupClicked(int index)
{
    Q_UNUSED(index)
    //ui->scheduleDraw->setCurrentIndex(index);
}

void ExposureSchedule::on_pushButton_default_clicked()
{
    ui->pushButton_autoMode->setChecked(true);
    on_pushButton_autoMode_clicked();
    ui->scheduleDraw->clearAll();
}

void ExposureSchedule::on_pushButton_selectAll_clicked()
{
    ui->scheduleDraw->selectAll();
}

void ExposureSchedule::on_pushButton_editTime_clicked()
{
    m_schedultEdit->setManualModeMap(m_mapAllManualMode);
    m_schedultEdit->setScheduleArray(ui->scheduleDraw->scheduleArray());
    m_schedultEdit->exec();
}

void ExposureSchedule::on_pushButton_ok_clicked()
{
    ui->scheduleDraw->getSchedule(m_exposureShce->schedule_day);
    emit scheduleAccept();
    reject();
}

void ExposureSchedule::on_pushButton_cancel_clicked()
{
    resetCache();
    reject();
}

void ExposureSchedule::apply()
{
    struct exposure_schedule data;
    memset(&data, 0, sizeof(struct exposure_schedule));
    data.chanid = m_channel;
    getSchedule(data.schedule_day);

    sendMessageOnly(REQUEST_FLAG_SET_EXPOSURE_SCHE, &data, sizeof(struct exposure_schedule));
}

void ExposureSchedule::on_pushButton_autoMode_clicked()
{
    ExposureManualValue value(ExposureActionAuto, 0, 0);
    ui->scheduleDraw->setCurrentType(value);
}

void ExposureSchedule::on_pushButton_manualMod_clicked()
{
    on_comboBox_manualModeEvent_activated(ui->comboBox_manualModeEvent->currentIndex());
}

void ExposureSchedule::on_comboBox_manualModeEvent_activated(int index)
{
    if (!m_mapAllManualMode.isEmpty() && !ui->pushButton_autoMode->isChecked()) {
        const ExposureManualValue &value = m_mapAllManualMode.value(index);
        qDebug() << QString("ExposureManualValue, exposureTime: %1, gainLevel: %2").arg(value.exposureTime).arg(value.gainLevel);
        ui->scheduleDraw->setCurrentType(value);
    } else {
        on_pushButton_autoMode_clicked();
    }
}

void ExposureSchedule::setExposureShce(exposure_schedule *exposureShce)
{
    if (!exposureShce) {
        return;
    }
    if (!m_exposureShce) {
        m_exposureShce = new exposure_schedule;
    }
    memset(m_exposureShce, 0, sizeof(struct exposure_schedule));
    memcpy(m_exposureShce, exposureShce, sizeof(struct exposure_schedule));
    setSchedule(m_exposureShce->schedule_day);
}

exposure_schedule *ExposureSchedule::getExposureShce() const
{
    ui->scheduleDraw->getSchedule(m_exposureShce->schedule_day);
    return m_exposureShce;
}

int ExposureSchedule::getChannel() const
{
    return m_channel;
}
