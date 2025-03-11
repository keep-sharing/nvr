#include "PeopleCountingInformationEdit.h"
#include "ui_PeopleCountingInformationEdit.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "centralmessage.h"

PeopleCountingInformationEdit::PeopleCountingInformationEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::PeopleCountingInformationEdit)
{
    ui->setupUi(this);
    ui->widgetCountTypeGroup->setCount(4);
    QStringList typeList;
    typeList << GET_TEXT("PEOPLECOUNTING_SEARCH/145017", "In")
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145018", "Out")
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145019", "Sum")
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145020", "Capacity");
    ui->widgetCountTypeGroup->setCheckBoxTest(typeList);

    ui->comboBoxOSDEnabled->beginEdit();
    ui->comboBoxOSDEnabled->clear();
    ui->comboBoxOSDEnabled->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxOSDEnabled->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxOSDEnabled->endEdit();

    ui->comboBoxLIneCountingInformation->beginEdit();
    ui->comboBoxLIneCountingInformation->clear();
    ui->comboBoxLIneCountingInformation->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxLIneCountingInformation->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxLIneCountingInformation->endEdit();

    ui->comboBoxAutoReset->beginEdit();
    ui->comboBoxAutoReset->clear();
    ui->comboBoxAutoReset->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxAutoReset->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxAutoReset->endEdit();

    ui->comboBoxDay->beginEdit();
    ui->comboBoxDay->clear();
    ui->comboBoxDay->addItem(GET_TEXT("AUTOREBOOT/78001", "Everyday"), 7);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1024", "Sunday"), 0);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1025", "Monday"), 1);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1026", "Tuesday"), 2);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1027", "Wednesday"), 3);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1028", "Thursday"), 4);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1029", "Friday"), 5);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1030", "Saturday"), 6);
    ui->comboBoxDay->endEdit();

    ui->comboBoxOSDTextPosition->beginEdit();
    ui->comboBoxOSDTextPosition->clear();
    ui->comboBoxOSDTextPosition->addItem(GET_TEXT("IMAGE/37335", "Top-Left"), 0);
    ui->comboBoxOSDTextPosition->addItem(GET_TEXT("IMAGE/37336", "Top-Right"), 1);
    ui->comboBoxOSDTextPosition->addItem(GET_TEXT("IMAGE/37337", "Bottom-Left"), 2);
    ui->comboBoxOSDTextPosition->addItem(GET_TEXT("IMAGE/37338", "Bottom-Right"), 3);
    ui->comboBoxOSDTextPosition->endEdit();

    m_peopleCountInfo = new ms_smart_event_people_cnt;
    m_peopleCountInfoCache = new ms_smart_event_people_cnt;

    onLanguageChanged();
}

PeopleCountingInformationEdit::~PeopleCountingInformationEdit()
{
    delete ui;
}

void PeopleCountingInformationEdit::initializeData(ms_smart_event_people_cnt *peopleCountInfo, int lineIndex)
{
    m_currentLine = lineIndex;
    memset(m_peopleCountInfo, 0, sizeof(ms_smart_event_people_cnt));
    memcpy(m_peopleCountInfo, peopleCountInfo, sizeof(ms_smart_event_people_cnt));
    m_peopleCountInfoCache = peopleCountInfo;
    ui->widgetCountTypeGroup->setCheckedFromInt(osdTypeChange(m_peopleCountInfo->osdType));
    ui->comboBoxOSDEnabled->setCurrentIndexFromData(m_peopleCountInfo->show_osd_enable);

    ui->comboBoxOSDFontSize->beginEdit();
    ui->comboBoxOSDFontSize->clear();
    ui->comboBoxOSDFontSize->addItem(GET_TEXT("PEOPLECOUNTING_SEARCH/145015", "Smallest"), 0);
    ui->comboBoxOSDFontSize->addItem(GET_TEXT("POS/130014", "Small"), 1);
    ui->comboBoxOSDFontSize->addItem(GET_TEXT("POS/130015", "Medium"), 2);
    ui->comboBoxOSDFontSize->addItem(GET_TEXT("POS/130016", "Large"), 3);
    if (m_peopleCountInfo->supportOsdLarger) {
        ui->comboBoxOSDFontSize->addItem(GET_TEXT("PEOPLECOUNTING_SEARCH/145016", "Largest"), 4);
    }
    ui->comboBoxOSDFontSize->addItem(GET_TEXT("SNAPSHOT/95026", "Auto"), 5);
    ui->comboBoxOSDFontSize->endEdit();
    ui->comboBoxOSDFontSize->setCurrentIndexFromData(m_peopleCountInfo->osd_font_size);

    ui->comboBoxOSDTextPosition->setCurrentIndexFromData(m_peopleCountInfo->osd_text_position);
    ui->comboBoxLIneCountingInformation->setCurrentIndexFromData(m_peopleCountInfo->lines[m_currentLine].showCntEnable);
    ui->comboBoxAutoReset->setCurrentIndexFromData(m_peopleCountInfo->lines[m_currentLine].autoResetEnable);
    ui->comboBoxDay->setCurrentIndexFromData(m_peopleCountInfo->lines[m_currentLine].autoResetWeekday);
    QTime time(m_peopleCountInfo->lines[m_currentLine].autoResetHour, m_peopleCountInfo->lines[m_currentLine].autoResetMin, m_peopleCountInfo->lines[m_currentLine].autoResetSec);
    ui->timeEdit->setTime(time);

    ui->toolButtonResetCountingInformation->setChecked(false);
    ui->toolButtonResetCountingData->setChecked(false);
    on_toolButtonResetCountingData_clicked(false);
    on_toolButtonResetCountingInformation_clicked(false);
}

void PeopleCountingInformationEdit::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_VAC_CLEANCOUNT:
        ON_RESPONSE_FLAG_SET_VAC_CLEANCOUNT(message);
        break;
    default:
        break;
    }
}

void PeopleCountingInformationEdit::ON_RESPONSE_FLAG_SET_VAC_CLEANCOUNT(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit();
}

quint64 PeopleCountingInformationEdit::osdTypeChange(quint64 osdType)
{
    quint64 sum = (osdType & (1 << 3)) >> 1;
    quint64 capacity = (osdType & (1 << 2)) << 1;
    osdType &= 3;
    osdType |= sum;
    osdType |= capacity;
    return osdType;
}

void PeopleCountingInformationEdit::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145006", "Counting Information Edit"));
    ui->labelCountType->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145007", "Count Type"));
    ui->labelOSDEnabled->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145008", "OSD"));
    ui->labelOSDFontSize->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145009", "OSD Font Size"));
    ui->labelOSDTextPosition->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145010", "OSD Text Position"));
    ui->labelLineCountingInformation->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145011", "Line Counting Information"));
    ui->labelResetCountingInformation->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145012", "Reset Counting Information"));
    ui->labelResetCountingData->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145013", "Reset Counting Data "));
    ui->labelAutoReset->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145021", "AUTO Reset"));
    ui->labelDay->setText(GET_TEXT("OCCUPANCY/74226", "Day"));
    ui->labelTime->setText(GET_TEXT("OCCUPANCY/74227", "Time"));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->pushButtonResetCountingData->setText(GET_TEXT("OCCUPANCY/74221", "Reset"));
    ui->pushButtonResetCountingInformation->setText(GET_TEXT("OCCUPANCY/74221", "Reset"));

    ui->labelResetCountingInformationTip->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145024", "Reset counting information in live view."));
    ui->labelResetCountingDataTip->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145025", "Reset counting information and statistics report."));
}

void PeopleCountingInformationEdit::on_pushButtonOk_clicked()
{
    m_peopleCountInfo->osdType = static_cast<Uint8>(osdTypeChange(ui->widgetCountTypeGroup->checkedFlags()));
    m_peopleCountInfo->show_osd_enable = ui->comboBoxOSDEnabled->currentIntData();
    m_peopleCountInfo->osd_font_size = ui->comboBoxOSDFontSize->currentIntData();
    m_peopleCountInfo->osd_text_position = ui->comboBoxOSDTextPosition->currentIntData();
    m_peopleCountInfo->lines[m_currentLine].showCntEnable = static_cast<Uint8>(ui->comboBoxLIneCountingInformation->currentIntData());
    m_peopleCountInfo->lines[m_currentLine].autoResetEnable = static_cast<Uint8>(ui->comboBoxAutoReset->currentIntData());
    m_peopleCountInfo->lines[m_currentLine].autoResetWeekday = ui->comboBoxDay->currentIntData();

    QTime time = ui->timeEdit->time();
    m_peopleCountInfo->lines[m_currentLine].autoResetHour = time.hour();
    m_peopleCountInfo->lines[m_currentLine].autoResetMin = time.minute();
    m_peopleCountInfo->lines[m_currentLine].autoResetSec = time.second();

    memset(m_peopleCountInfoCache, 0, sizeof(ms_smart_event_people_cnt));
    memcpy(m_peopleCountInfoCache, m_peopleCountInfo, sizeof(ms_smart_event_people_cnt));
    accept();
}

void PeopleCountingInformationEdit::on_pushButtonCancel_clicked()
{
    reject();
}

void PeopleCountingInformationEdit::on_pushButtonResetCountingInformation_clicked()
{
    const int &result = MessageBox::question(this, GET_TEXT("PEOPLECOUNTING_SEARCH/145014", "Do you really want to reset？"));
    if (result == MessageBox::Yes) {
        ms_vca_cleancount_info lineInfo;
        memset(&lineInfo, 0, sizeof(ms_vca_cleancount_info));
        lineInfo.line = m_currentLine;
        lineInfo.chanid = m_peopleCountInfo->chanid;
        sendMessage(REQUEST_FLAG_SET_VAC_CLEANCOUNT, &lineInfo, sizeof(ms_vca_cleancount_info));
        //m_eventLoop.exec();
    }
}

void PeopleCountingInformationEdit::on_pushButtonResetCountingData_clicked()
{
    const int &result = MessageBox::question(this, GET_TEXT("PEOPLECOUNTING_SEARCH/145014", "Do you really want to reset？"));
    if (result == MessageBox::Yes) {
        ms_vca_cleancount_info lineInfo;
        memset(&lineInfo, 0, sizeof(ms_vca_cleancount_info));
        lineInfo.line = m_currentLine;
        lineInfo.chanid = m_peopleCountInfo->chanid;
        lineInfo.resetdb = 1;
        sendMessage(REQUEST_FLAG_SET_VAC_CLEANCOUNT, &lineInfo, sizeof(ms_vca_cleancount_info));
        //m_eventLoop.exec();
    }
}

void PeopleCountingInformationEdit::on_toolButtonResetCountingInformation_clicked(bool checked)
{
    if (checked) {
        ui->toolButtonResetCountingInformation->setIcon(QIcon(":/common/common/note_hover.png"));
    } else {
        ui->toolButtonResetCountingInformation->setIcon(QIcon(":/common/common/note.png"));
    }
    ui->labelResetCountingInformationTip->setVisible(checked);
}

void PeopleCountingInformationEdit::on_toolButtonResetCountingData_clicked(bool checked)
{
    if (checked) {
        ui->toolButtonResetCountingData->setIcon(QIcon(":/common/common/note_hover.png"));
    } else {
        ui->toolButtonResetCountingData->setIcon(QIcon(":/common/common/note.png"));
    }
    ui->labelResetCountingDataTip->setVisible(checked);
}
