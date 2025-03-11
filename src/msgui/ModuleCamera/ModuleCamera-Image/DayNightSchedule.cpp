#include "DayNightSchedule.h"
#include "ui_DayNightSchedule.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QButtonGroup>
#include <QtDebug>

DayNightSchedule::DayNightSchedule(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::DayNightSchedule)
{
    ui->setupUi(this);
    setTitleWidget(ui->label_title);

    m_schedultEdit = new DayNightScheTimeEdit(this);
    connect(m_schedultEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    QButtonGroup *buttonGroup = new QButtonGroup(this);

    buttonGroup->addButton(ui->pushButtonTemplate1);
    buttonGroup->addButton(ui->pushButtonTemplate2);
    buttonGroup->addButton(ui->pushButtonTemplate3);
    buttonGroup->addButton(ui->pushButtonTemplate4);
    buttonGroup->addButton(ui->pushButtonTemplate5);
    buttonGroup->addButton(ui->pushButtonErase);

    ui->scheduleDraw->setScheduleMode(MsScheduleDraw::Mode_OnlySunday);
    ui->scheduleDraw->setTypeColor(ACTION_TEMPLATE1, QColor("#2EB8E6"));
    ui->scheduleDraw->setTypeColor(ACTION_TEMPLATE2, QColor("#800080"));
    ui->scheduleDraw->setTypeColor(ACTION_TEMPLATE3, QColor("#1BC15B"));
    ui->scheduleDraw->setTypeColor(ACTION_TEMPLATE4, QColor("#FF9900"));
    ui->scheduleDraw->setTypeColor(ACTION_TEMPLATE5, QColor("#FF96FF"));
    ui->scheduleDraw->setTypeColor(ACTION_ERASE, QColor("#DEDEDE"));

    onLanguageChanged();
}

DayNightSchedule::~DayNightSchedule()
{
    delete ui;
}

void DayNightSchedule::setSchedule(schedule_day *schedule_day_array)
{
    ui->scheduleDraw->setSchedule(schedule_day_array);
}

void DayNightSchedule::getSchedule(schedule_day *schedule_day_array)
{
    ui->scheduleDraw->getSchedule(schedule_day_array);
}

void DayNightSchedule::showAction(int channel, set_image_day_night_str *info)
{
    m_channel = channel;
    m_info = info;
    memset(m_dayNightShce, 0, sizeof(schedule_day) * MAX_DAY_NUM);
    if (!m_info->imgSingle.hasSche) {
        changeToScheduleDay();
    } else {
        //type 0在qt中是绘制空，template1~5的type为0~4 。因此在数据中使用1~5。
        for (int j = 0; j < IPC_SCHE_TEN_SECTIONS; j++) {
            if (QString(m_info->imgSingle.dnSche[j].start_time).isEmpty() || QString(m_info->imgSingle.dnSche[j].end_time).isEmpty()) {
                continue;
            }
            m_info->imgSingle.dnSche[j].action_type++;
        }
        memcpy(m_dayNightShce[0].schedule_item, m_info->imgSingle.dnSche, sizeof(schdule_item) * IPC_SCHE_TEN_SECTIONS);
    }
    setSchedule(m_dayNightShce);

    ui->pushButtonErase->setChecked(true);
    on_pushButtonErase_clicked();
}

void DayNightSchedule::changeToScheduleDay()
{
    uint16_t secArray[86401] { 0 };
    for (int i = 2; i < MAX_IMAGE_DAY_NIGHT; i++) {
        if (!m_info->imgSingle.image[i].enable) {
            continue;
        }
        int start = (m_info->imgSingle.image[i].startHour * 3600) + (m_info->imgSingle.image[i].startMinute * 60);
        int end = (m_info->imgSingle.image[i].endHour * 3600) + (m_info->imgSingle.image[i].endMinute * 60);
        for (int j = start; j <= end; j++) {
            secArray[j] = static_cast<uint16_t>(i - 1);
        }
    }
    schdule_item *item_array = m_dayNightShce[0].schedule_item;
    QList<ScheduleTime> times = ScheduleTime::fromSecondsHash(secArray);
    int index = 0;
    for (int j = 0; j < times.size(); ++j) {
        const ScheduleTime &time = times.at(j);

        schdule_item &s_item = item_array[index];
        s_item.action_type = time.type();
        QString strBegin = QString("%1:%2").arg(time.beginMinute() / 60, 2, 10, QLatin1Char('0')).arg(time.beginMinute() % 60, 2, 10, QLatin1Char('0'));
        QString strEnd = QString("%1:%2").arg(time.endMinute() / 60, 2, 10, QLatin1Char('0')).arg(time.endMinute() % 60, 2, 10, QLatin1Char('0'));
        snprintf(s_item.start_time, sizeof(s_item.start_time), "%s", strBegin.toStdString().c_str());
        snprintf(s_item.end_time, sizeof(s_item.end_time), "%s", strEnd.toStdString().c_str());

        index++;
    }
}

void DayNightSchedule::changeToImageStr()
{
    QList<ScheduleTime> times = ScheduleTime::fromSecondsHash(ui->scheduleDraw->scheduleArray()[0]);
    for (int i = 2; i < MAX_IMAGE_DAY_NIGHT; i++) {
        bool findTime = false;
        for (auto item = times.constBegin(); item != times.constEnd(); item++) {
            if (item->type() == i - 1) {
                m_info->imgSingle.image[i].startHour = item->beginMinute() / 60;
                m_info->imgSingle.image[i].startMinute = item->beginMinute() % 60;
                m_info->imgSingle.image[i].endHour = item->endMinute() / 60;
                m_info->imgSingle.image[i].endMinute = item->endMinute() % 60;
                findTime = true;
                break;
            }
        }
        m_info->imgSingle.image[i].enable = findTime;
        if (!findTime) {
            m_info->imgSingle.image[i].startHour = 0;
            m_info->imgSingle.image[i].startMinute = 0;
            m_info->imgSingle.image[i].endHour = 0;
            m_info->imgSingle.image[i].endMinute = 0;
        }
    }
}

void DayNightSchedule::onLanguageChanged()
{
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    ui->pushButton_default->setText(GET_TEXT("COMMON/1050", "Default"));
    ui->pushButton_selectAll->setText(GET_TEXT("COMMON/1022", "Select All"));
    ui->pushButton_editTime->setText(GET_TEXT("RECORDMODE/90010", "Edit Time"));

    ui->pushButtonTemplate1->setText(GET_TEXT("IMAGE/162001", "Template") + "1");
    ui->pushButtonTemplate2->setText(GET_TEXT("IMAGE/162001", "Template") + "2");
    ui->pushButtonTemplate3->setText(GET_TEXT("IMAGE/162001", "Template") + "3");
    ui->pushButtonTemplate4->setText(GET_TEXT("IMAGE/162001", "Template") + "4");
    ui->pushButtonTemplate5->setText(GET_TEXT("IMAGE/162001", "Template") + "5");
    ui->pushButtonErase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
}

void DayNightSchedule::onEditingFinished()
{
    ui->scheduleDraw->setScheduleArray(m_schedultEdit->scheduleArray());
}

void DayNightSchedule::on_pushButton_default_clicked()
{
    ui->pushButtonErase->setChecked(true);
    on_pushButtonErase_clicked();
    ui->scheduleDraw->selectAll();
}

void DayNightSchedule::on_pushButton_selectAll_clicked()
{
    ui->scheduleDraw->selectAll();
}

void DayNightSchedule::on_pushButton_editTime_clicked()
{
    m_schedultEdit->setScheduleArray(ui->scheduleDraw->scheduleArray());
    m_schedultEdit->exec();
}

void DayNightSchedule::on_pushButton_ok_clicked()
{
    getSchedule(m_dayNightShce);
    if (m_info->imgSingle.hasSche) {
        memcpy(m_info->imgSingle.dnSche, m_dayNightShce[0].schedule_item, sizeof(schdule_item) * IPC_SCHE_TEN_SECTIONS);
        for (int j = 0; j < IPC_SCHE_TEN_SECTIONS; j++) {
            if (QString(m_info->imgSingle.dnSche[j].start_time).isEmpty() || QString(m_info->imgSingle.dnSche[j].end_time).isEmpty()) {
                continue;
            }
            m_info->imgSingle.dnSche[j].action_type--;
        }
    }
    changeToImageStr();

    accept();
}

void DayNightSchedule::on_pushButton_cancel_clicked()
{
    if (m_info->imgSingle.hasSche) {
        for (int j = 0; j < IPC_SCHE_TEN_SECTIONS; j++) {
            if (QString(m_info->imgSingle.dnSche[j].start_time).isEmpty() || QString(m_info->imgSingle.dnSche[j].end_time).isEmpty()) {
                continue;
            }
            m_info->imgSingle.dnSche[j].action_type--;
        }
    }
    reject();
}

void DayNightSchedule::on_pushButtonTemplate1_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_TEMPLATE1);
}

void DayNightSchedule::on_pushButtonTemplate2_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_TEMPLATE2);
}

void DayNightSchedule::on_pushButtonTemplate3_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_TEMPLATE3);
}

void DayNightSchedule::on_pushButtonTemplate4_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_TEMPLATE4);
}

void DayNightSchedule::on_pushButtonTemplate5_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_TEMPLATE5);
}

void DayNightSchedule::on_pushButtonErase_clicked()
{
    ui->scheduleDraw->setCurrentType(ACTION_ERASE);
}
