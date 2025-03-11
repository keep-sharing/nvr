#include "EffectiveTimeAbstract.h"
#include "ui_EffectiveTimeAbstract.h"
#include "MsLanguage.h"

EffectiveTimeAbstract::EffectiveTimeAbstract(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::EffectiveTimeAbstract)
{
    ui->setupUi(this);

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->pushButtonEffective);
    buttonGroup->addButton(ui->pushButtonErase);
}

EffectiveTimeAbstract::~EffectiveTimeAbstract()
{
    delete ui;
}

void EffectiveTimeAbstract::showEffectiveTime(int channel, int dataIndex)
{
    m_channel = channel;
    m_dataIndex = dataIndex;

    ui->labelEffective->setColor(scheduleColor());
    ui->schedule->setHolidayVisible(holidayVisible());
    ui->schedule->setCurrentType(scheduleType());
    ui->schedule->setTypeColor(scheduleType(), scheduleColor());
    ui->schedule->setSingleEditType(scheduleType());
    ui->schedule->setSchedule(schedule());

    ui->pushButtonEffective->setChecked(true);

    onLanguageChanged();
    show();
}

void EffectiveTimeAbstract::saveEffectiveTime()
{
    saveSchedule();
}

void EffectiveTimeAbstract::saveEffectiveTime(int dataIndex)
{
    saveSchedule(dataIndex);
}

void EffectiveTimeAbstract::clearCache()
{
    m_dataMap.clear();
}

void EffectiveTimeAbstract::onLanguageChanged()
{
    ui->label_title->setText(titleText());

    ui->pushButtonEffective->setText(pushButtonEffectiveText());
    ui->pushButtonErase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));

    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

QString EffectiveTimeAbstract::titleText() const
{
    return GET_TEXT("SMARTEVENT/55015", "Effective Time");
}

QString EffectiveTimeAbstract::pushButtonEffectiveText() const
{
    return GET_TEXT("SMARTEVENT/55015", "Effective Time");
}

int EffectiveTimeAbstract::channel() const
{
    return m_channel;
}

int EffectiveTimeAbstract::dataIndex() const
{
    return m_dataIndex;
}

bool EffectiveTimeAbstract::holidayVisible()
{
    return true;
}

schedule_day *EffectiveTimeAbstract::schedule()
{
    if (m_dataMap.contains(dataIndex())) {
        auto &data = m_dataMap[dataIndex()];
        return data.schedule.schedule_day;
    } else {
        return readSchedule();
    }
}

void EffectiveTimeAbstract::saveSchedule(int dataIndex)
{
    Q_UNUSED(dataIndex)
}

void EffectiveTimeAbstract::on_pushButtonEffective_clicked()
{
    ui->schedule->setCurrentType(scheduleType());
}

void EffectiveTimeAbstract::on_pushButtonErase_clicked()
{
    ui->schedule->setCurrentType(NONE);
}

void EffectiveTimeAbstract::on_pushButtonOk_clicked()
{
    ui->schedule->getSchedule(schedule());
    accept();
}

void EffectiveTimeAbstract::on_pushButtonCancel_clicked()
{
    reject();
}

void EffectiveTimeAbstract::setDataIndex(int dataIndex)
{
    m_dataIndex = dataIndex;
}

void EffectiveTimeAbstract::setChannel(int channel)
{
    m_channel = channel;
}
