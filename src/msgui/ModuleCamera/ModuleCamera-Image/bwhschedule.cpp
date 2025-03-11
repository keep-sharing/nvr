#include "bwhschedule.h"
#include "ui_bwhschedule.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QButtonGroup>
#include <QtDebug>

BWHSchedule::BWHSchedule(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::BWHSchedule)
{
    ui->setupUi(this);
    setTitleWidget(ui->label_title);

    m_schedultEdit = new ImageScheduleEdit(this);
    connect(m_schedultEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    QButtonGroup *buttonGroup = new QButtonGroup(this);

    buttonGroup->addButton(ui->pushButton_BLC, ACTION_BLC);
    buttonGroup->addButton(ui->pushButton_WDR, ACTION_WDR);
    buttonGroup->addButton(ui->pushButton_HLC, ACTION_HLC);

    ui->scheduleDraw->setScheduleMode(MsScheduleDraw::Mode_NoHoliday);
    ui->scheduleDraw->setTypeColor(ACTION_BLC, QColor("#C8C8C8"));
    ui->scheduleDraw->setTypeColor(ACTION_WDR, QColor("#1BC15B"));
    ui->scheduleDraw->setTypeColor(ACTION_HLC, QColor("#2EB8E6"));

    m_schedultEdit->clearType();
    m_schedultEdit->addType(QString("BLC"), ACTION_BLC);
    m_schedultEdit->addType(QString("WDR"), ACTION_WDR);
    m_schedultEdit->addType(QString("HLC"), ACTION_HLC);

    onLanguageChanged();
}

BWHSchedule::~BWHSchedule()
{
    if (m_BlcWdrHlcShce) {
        delete m_BlcWdrHlcShce;
        m_BlcWdrHlcShce = nullptr;
    }
    delete ui;
}

void BWHSchedule::showAction(int channel)
{
    m_channel = channel;

    ui->pushButton_BLC->setChecked(true);
    on_pushButton_BLC_clicked();
}

void BWHSchedule::ON_RESPONSE_FLAG_GET_BWH_SCHE(MessageReceive *message)
{
    struct bwh_schedule *data = (struct bwh_schedule *)message->data;
    if (!data) {
        qWarning() << "BWHSchedule::ON_RESPONSE_FLAG_GET_BWH_SCHE, data is null.";
        return;
    }
    if (!m_BlcWdrHlcShce) {
        m_BlcWdrHlcShce = new bwh_schedule;
    }
    memcpy(m_BlcWdrHlcShce, data, sizeof(struct bwh_schedule));
    setSchedule(m_BlcWdrHlcShce->schedule_day);
}

void BWHSchedule::setSchedule(schedule_day *schedule_day_array)
{
    ui->scheduleDraw->setSchedule(schedule_day_array);
}

void BWHSchedule::getSchedule(schedule_day *schedule_day_array)
{
    ui->scheduleDraw->getSchedule(schedule_day_array);
}

void BWHSchedule::onEditingFinished()
{
    ui->scheduleDraw->setScheduleArray(m_schedultEdit->scheduleArray());
}

void BWHSchedule::onButtonGroupClicked(int index)
{
    Q_UNUSED(index)
    //ui->scheduleDraw->setCurrentIndex(index);
}

void BWHSchedule::on_pushButton_default_clicked()
{
    ui->pushButton_BLC->setChecked(true);
    on_pushButton_BLC_clicked();
    ui->scheduleDraw->selectAll();
}

void BWHSchedule::on_pushButton_selectAll_clicked()
{
    ui->scheduleDraw->selectAll();
}

void BWHSchedule::on_pushButton_editTime_clicked()
{
    m_schedultEdit->setScheduleArray(ui->scheduleDraw->scheduleArray());
    m_schedultEdit->exec();
}

void BWHSchedule::on_pushButton_ok_clicked()
{
    reject();
}

void BWHSchedule::on_pushButton_cancel_clicked()
{
    reject();
}

void BWHSchedule::apply()
{
    struct bwh_schedule data;
    memset(&data, 0, sizeof(struct bwh_schedule));
    data.chanid = m_channel;
    getSchedule(data.schedule_day);

    sendMessageOnly(REQUEST_FLAG_SET_BWH_SCHE, &data, sizeof(struct bwh_schedule));
}

void BWHSchedule::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("IMAGE/37227", "BLC/WDR/HLC Schedule"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    ui->pushButton_default->setText(GET_TEXT("COMMON/1050", "Default"));
    ui->pushButton_selectAll->setText(GET_TEXT("COMMON/1022", "Select All"));
    ui->pushButton_editTime->setText(GET_TEXT("COMMON/1019", "Edit"));
}

void BWHSchedule::on_pushButton_WDR_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_WDR);
}

void BWHSchedule::on_pushButton_HLC_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_HLC);
}

void BWHSchedule::on_pushButton_BLC_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_BLC);
}
