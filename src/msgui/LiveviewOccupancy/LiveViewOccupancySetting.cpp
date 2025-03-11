#include "LiveViewOccupancySetting.h"
#include "ui_LiveViewOccupancySetting.h"
#include "LiveViewOccupancyManager.h"
#include "MsLanguage.h"
#include "PeopleCountingData.h"
#include <QFile>
#include "MyDebug.h"
#include "centralmessage.h"
#include "MessageBox.h"

LiveViewOccupancySetting::LiveViewOccupancySetting(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::LiveViewOccupancySetting)
{
    ui->setupUi(this);

    loadStylesheet(":/style/style/settingstyle.qss");

    connect(ui->comboBoxGroup, SIGNAL(indexSet(int)), this, SLOT(onComboBoxGroupIndexSet(int)));

    connect(ui->checkBoxAvailable, SIGNAL(stateChanged(int)), this, SLOT(onDisplayStateChanged()));
    connect(ui->checkBoxStay, SIGNAL(stateChanged(int)), this, SLOT(onDisplayStateChanged()));
    connect(ui->checkBoxIn, SIGNAL(stateChanged(int)), this, SLOT(onDisplayStateChanged()));
    connect(ui->checkBoxOut, SIGNAL(stateChanged(int)), this, SLOT(onDisplayStateChanged()));

    onLanguageChanged();
}

LiveViewOccupancySetting::~LiveViewOccupancySetting()
{
    delete ui;
}

void LiveViewOccupancySetting::show()
{
    int groupCount = 0;
    ui->comboBoxGroup->clear();
    m_lastGroup = 0;
    people_cnt_info *people_info = gPeopleCountingData.peopleInfo();
    for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
        const PEOPLECNT_SETTING &setting = people_info->sets[i];
        if (PeopleCountingData::hasChannel(setting.tri_channels)) {
            QString name(setting.name);
            if (name.isEmpty()) {
                name = QString("Group %1").arg(i + 1);
            }
            ui->comboBoxGroup->addItem(QString("%1").arg(name), i);
            groupCount++;
        }
        //
        m_displayMap.insert(i, setting.liveview_display);
    }
    if (groupCount > 0) {
        int group = LiveViewOccupancyManager::instance()->group();
        ui->comboBoxGroup->setCurrentIndexFromData(group);
        ui->checkBoxAvailable->setEnabled(true);
        ui->checkBoxStay->setEnabled(true);
        ui->checkBoxIn->setEnabled(true);
        ui->checkBoxOut->setEnabled(true);
    } else {
        m_lastGroup = -1;
        ui->checkBoxAvailable->setChecked(true);
        ui->checkBoxStay->setChecked(true);
        ui->checkBoxIn->setChecked(true);
        ui->checkBoxOut->setChecked(true);
        ui->checkBoxAvailable->setEnabled(false);
        ui->checkBoxStay->setEnabled(false);
        ui->checkBoxIn->setEnabled(false);
        ui->checkBoxOut->setEnabled(false);
    }
    //
    BaseShadowDialog::show();
}

int LiveViewOccupancySetting::currentDisplayState() const
{
    int group = ui->comboBoxGroup->currentIntData();
    return m_displayMap.value(group);
}

int LiveViewOccupancySetting::currentGroup() const
{
    int group = ui->comboBoxGroup->currentIntData();
    return group;
}

bool LiveViewOccupancySetting::checkDisplay(int display)
{
    if (display & PEOPLECNT_DISPLAY_AVALIABLE) {
        return true;
    }
    if (display & PEOPLECNT_DISPLAY_STAY) {
        return true;
    }
    if (display & PEOPLECNT_DISPLAY_IN) {
        return true;
    }
    if (display & PEOPLECNT_DISPLAY_OUT) {
        return true;
    }
    return false;
}

void LiveViewOccupancySetting::onLanguageChanged()
{
    ui->labelDisplayedItem->setText(GET_TEXT("OCCUPANCY/74262", "Displayed Items"));
    ui->checkBoxAvailable->setText(GET_TEXT("OCCUPANCY/74267", "Available"));
    ui->checkBoxStay->setText(GET_TEXT("OCCUPANCY/74266", "Stay"));
    ui->checkBoxIn->setText(GET_TEXT("OCCUPANCY/74264", "In"));
    ui->checkBoxOut->setText(GET_TEXT("OCCUPANCY/74265", "Out"));

    ui->label_title->setText(GET_TEXT("LIVEVIEW/20065", "Display Settings"));
    ui->labelGroup->setText(GET_TEXT("OCCUPANCY/74223", "Real-time Display"));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void LiveViewOccupancySetting::onComboBoxGroupIndexSet(int index)
{
    int group = ui->comboBoxGroup->itemData(index).toInt();
    int display = m_displayMap.value(group);
    int previousDisplay = m_displayMap.value(m_lastGroup);
    if (!checkDisplay(previousDisplay)) {
        if (m_lastGroup >= 0) {
            ui->comboBoxGroup->editCurrentIndexFromData(m_lastGroup);
        }
        MessageBox::information(this, GET_TEXT("OCCUPANCY/74263", "Please select at least one displayed item."));
        return;
    }
    m_lastGroup = group;
    ui->checkBoxAvailable->setChecked(display & PEOPLECNT_DISPLAY_AVALIABLE);
    ui->checkBoxStay->setChecked(display & PEOPLECNT_DISPLAY_STAY);
    ui->checkBoxIn->setChecked(display & PEOPLECNT_DISPLAY_IN);
    ui->checkBoxOut->setChecked(display & PEOPLECNT_DISPLAY_OUT);
}

void LiveViewOccupancySetting::onDisplayStateChanged()
{
    int group = ui->comboBoxGroup->currentIntData();
    int &display = m_displayMap[group];
    if (ui->checkBoxAvailable->isChecked()) {
        display |= PEOPLECNT_DISPLAY_AVALIABLE;
    } else {
        display &= ~PEOPLECNT_DISPLAY_AVALIABLE;
    }
    if (ui->checkBoxStay->isChecked()) {
        display |= PEOPLECNT_DISPLAY_STAY;
    } else {
        display &= ~PEOPLECNT_DISPLAY_STAY;
    }
    if (ui->checkBoxIn->isChecked()) {
        display |= PEOPLECNT_DISPLAY_IN;
    } else {
        display &= ~PEOPLECNT_DISPLAY_IN;
    }
    if (ui->checkBoxOut->isChecked()) {
        display |= PEOPLECNT_DISPLAY_OUT;
    } else {
        display &= ~PEOPLECNT_DISPLAY_OUT;
    }
}

void LiveViewOccupancySetting::on_pushButtonOk_clicked()
{
    int group = ui->comboBoxGroup->currentIntData();
    int display = m_displayMap.value(group);
    if (!checkDisplay(display)) {
        ui->pushButtonOk->clearUnderMouse();
        ui->comboBoxGroup->editCurrentIndexFromData(m_lastGroup);
        MessageBox::information(this, GET_TEXT("OCCUPANCY/74263", "Please select at least one displayed item."));
        return;
    }

    people_cnt_info *people_info = gPeopleCountingData.peopleInfo();
    for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
        PEOPLECNT_SETTING &setting = people_info->sets[i];
        setting.liveview_display = m_displayMap.value(i);
    }
    write_peoplecnt_settings(SQLITE_FILE_NAME, people_info->sets, MAX_PEOPLECNT_GROUP);
    sendMessage(REQUEST_FLAG_UPDATE_PEOPLECNT_SETTING, nullptr, 0);
    //
    LiveViewOccupancyManager::instance()->setGroup(group);
    accept();
}

void LiveViewOccupancySetting::on_pushButtonCancel_clicked()
{
    close();
}
