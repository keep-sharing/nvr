#include "whitebalanceschedule.h"
#include "ui_whitebalanceschedule.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QButtonGroup>
#include <QtDebug>
WhiteBalanceSchedule::WhiteBalanceSchedule(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::WhiteBalanceSchedule)
{
    ui->setupUi(this);
    setTitleWidget(ui->label_title);

    m_schedultEdit = new ImageScheduleEdit(this);
    connect(m_schedultEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    QButtonGroup *buttonGroup = new QButtonGroup(this);

    buttonGroup->addButton(ui->pushButton_ManualMode, ACTION_MANUALMODE);
    buttonGroup->addButton(ui->pushButton_Incandescent, ACTION_INCANDESCENT);
    buttonGroup->addButton(ui->pushButton_WarmLight, ACTION_WARMLIGHT);
    buttonGroup->addButton(ui->pushButton_NaturalLight, ACTION_NATURALLIGHT);
    buttonGroup->addButton(ui->pushButton_Fluorescent, ACTION_FLUORESCENT);
    buttonGroup->addButton(ui->pushButton_AutoMode, ACTION_AUTOMODE);

    ui->scheduleDraw->setScheduleMode(MsScheduleDraw::Mode_NoHoliday);
    ui->scheduleDraw->setTypeColor(ACTION_AUTOMODE, QColor("#DEDEDE"));
    ui->scheduleDraw->setTypeColor(ACTION_MANUALMODE, QColor("#2EB8E6"));
    ui->scheduleDraw->setTypeColor(ACTION_INCANDESCENT, QColor("#800080"));
    ui->scheduleDraw->setTypeColor(ACTION_WARMLIGHT, QColor("#1BC15B"));
    ui->scheduleDraw->setTypeColor(ACTION_NATURALLIGHT, QColor("#FF9900"));
    ui->scheduleDraw->setTypeColor(ACTION_FLUORESCENT, QColor("#FF96FF"));

    m_schedultEdit->clearType();
    m_schedultEdit->addType(GET_TEXT("IMAGE/37104", "Auto Mode"), ACTION_AUTOMODE);
    m_schedultEdit->addType(GET_TEXT("IMAGE/37203", "Manual Mode"), ACTION_MANUALMODE);
    m_schedultEdit->addType(GET_TEXT("IMAGE/37418", "Incandescent"), ACTION_INCANDESCENT);
    m_schedultEdit->addType(GET_TEXT("IMAGE/37419", "Warm Light"), ACTION_WARMLIGHT);
    m_schedultEdit->addType(GET_TEXT("IMAGE/37348", "Natural Light"), ACTION_NATURALLIGHT);
    m_schedultEdit->addType(GET_TEXT("IMAGE/37420", "Fluorescent"), ACTION_FLUORESCENT);

    onLanguageChanged();
}

WhiteBalanceSchedule::~WhiteBalanceSchedule()
{
    if (m_whiteBalanceShce) {
        delete m_whiteBalanceShce;
        m_whiteBalanceShce = nullptr;
    }
    delete ui;
}

void WhiteBalanceSchedule::showAction(int channel)
{
    m_channel = channel;

    ui->pushButton_AutoMode->setChecked(true);
    on_pushButton_AutoMode_clicked();
}

void WhiteBalanceSchedule::ON_RESPONSE_FLAG_GET_WHITE_BALANCE_SCHE(MessageReceive *message)
{
    struct white_balance_schedule *data = (struct white_balance_schedule *)message->data;
    if (!data) {
        qWarning() << "WhiteBalanceSchedule::ON_RESPONSE_FLAG_GET_WHITE_BALANCE_SCHE";
        return;
    }
    if (!m_whiteBalanceShce) {
        m_whiteBalanceShce = new white_balance_schedule;
    }
    memcpy(m_whiteBalanceShce, data, sizeof(struct white_balance_schedule));
    setSchedule(m_whiteBalanceShce->schedule_day);
}

void WhiteBalanceSchedule::setSchedule(schedule_day *schedule_day_array)
{
    ui->scheduleDraw->setSchedule(schedule_day_array);
}

void WhiteBalanceSchedule::getSchedule(schedule_day *schedule_day_array)
{
    ui->scheduleDraw->getSchedule(schedule_day_array);
}

void WhiteBalanceSchedule::onEditingFinished()
{
    ui->scheduleDraw->setScheduleArray(m_schedultEdit->scheduleArray());
}

void WhiteBalanceSchedule::onButtonGroupClicked(int index)
{
    Q_UNUSED(index)
    //ui->scheduleDraw->setCurrentIndex(index);
}

void WhiteBalanceSchedule::on_pushButton_default_clicked()
{
    ui->pushButton_AutoMode->setChecked(true);
    on_pushButton_AutoMode_clicked();
    ui->scheduleDraw->selectAll();
}

void WhiteBalanceSchedule::on_pushButton_selectAll_clicked()
{
    ui->scheduleDraw->selectAll();
}

void WhiteBalanceSchedule::on_pushButton_editTime_clicked()
{
    m_schedultEdit->setScheduleArray(ui->scheduleDraw->scheduleArray());
    m_schedultEdit->exec();
}

void WhiteBalanceSchedule::on_pushButton_ok_clicked()
{
    reject();
}

void WhiteBalanceSchedule::on_pushButton_cancel_clicked()
{
    reject();
}

void WhiteBalanceSchedule::apply()
{
    struct white_balance_schedule data;
    memset(&data, 0, sizeof(struct white_balance_schedule));
    data.chanid = m_channel;
    getSchedule(data.schedule_day);

    sendMessageOnly(REQUEST_FLAG_SET_WHITE_BALANCE_SCHE, &data, sizeof(struct white_balance_schedule));
}

void WhiteBalanceSchedule::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("IMAGE/37206", "White Balance Schedule"));
    ui->pushButton_default->setText(GET_TEXT("COMMON/1050", "Default"));
    ui->pushButton_selectAll->setText(GET_TEXT("COMMON/1022", "Select All"));
    ui->pushButton_editTime->setText(GET_TEXT("RECORDMODE/90010", "Edit Time"));

    ui->pushButton_ManualMode->setText(GET_TEXT("IMAGE/37203", "Manual Mode"));
    ui->pushButton_Incandescent->setText(GET_TEXT("IMAGE/37418", "Incandescent"));
    ui->pushButton_WarmLight->setText(GET_TEXT("IMAGE/37419", "Warm Light"));
    ui->pushButton_NaturalLight->setText(GET_TEXT("IMAGE/37348", "Natural Light"));
    ui->pushButton_Fluorescent->setText(GET_TEXT("IMAGE/37420", "Fluorescent"));
    ui->pushButton_AutoMode->setText(GET_TEXT("IMAGE/37104", "Auto Mode"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void WhiteBalanceSchedule::on_pushButton_ManualMode_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_MANUALMODE);
}

void WhiteBalanceSchedule::on_pushButton_Incandescent_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_INCANDESCENT);
}

void WhiteBalanceSchedule::on_pushButton_WarmLight_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_WARMLIGHT);
}

void WhiteBalanceSchedule::on_pushButton_NaturalLight_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_NATURALLIGHT);
}

void WhiteBalanceSchedule::on_pushButton_Fluorescent_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_FLUORESCENT);
}

void WhiteBalanceSchedule::on_pushButton_AutoMode_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_AUTOMODE);
}
