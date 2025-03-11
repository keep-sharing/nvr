#include "TabOccupancyLiveViewSettings.h"
#include "ui_TabOccupancyLiveViewSettings.h"
#include "ActionOccupancy.h"
#include "LiveViewOccupancyManager.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "OccupancyGroupSettings.h"
#include "PeopleCountingData.h"
#include "centralmessage.h"

extern "C" {

#include "msg.h"
}

TabOccupancyLiveViewSettings::TabOccupancyLiveViewSettings(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabOccupancyLiveViewSettings)
{
    ui->setupUi(this);

    //
    ui->comboBoxPeopleCounting->beginEdit();
    ui->comboBoxPeopleCounting->clear();
    ui->comboBoxPeopleCounting->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxPeopleCounting->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxPeopleCounting->endEdit();

    //
    ui->comboBoxFontSize->beginEdit();
    ui->comboBoxFontSize->clear();
    ui->comboBoxFontSize->addItem(GET_TEXT("OCCUPANCY/74229", "Small"), 0);
    ui->comboBoxFontSize->addItem(GET_TEXT("OCCUPANCY/74230", "Medium"), 1);
    ui->comboBoxFontSize->addItem(GET_TEXT("OCCUPANCY/74231", "Large"), 2);
    ui->comboBoxFontSize->endEdit();

    //
    ui->comboBoxLiveViewCountingAutoReset->beginEdit();
    ui->comboBoxLiveViewCountingAutoReset->clear();
    ui->comboBoxLiveViewCountingAutoReset->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxLiveViewCountingAutoReset->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxLiveViewCountingAutoReset->endEdit();

    //
    ui->comboBoxDay->beginEdit();
    ui->comboBoxDay->clear();
    ui->comboBoxDay->addItem(GET_TEXT("AUTOREBOOT/78001", "Everyday"), 0);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1024", "Sunday"), 1);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1025", "Monday"), 2);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1026", "Tuesday"), 3);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1027", "Wednesday"), 4);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1028", "Thursday"), 5);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1029", "Friday"), 6);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1030", "Saturday"), 7);
    ui->comboBoxDay->endEdit();

    ui->lineEditMaxStays->setCheckMode(MyLineEdit::RangeCheck, 1, 99999);

    onLanguageChanged();
}

TabOccupancyLiveViewSettings::~TabOccupancyLiveViewSettings()
{
    delete ui;
}

void TabOccupancyLiveViewSettings::initializeData()
{

    ui->toolButtonNote1->setChecked(false);
    on_toolButtonNote1_clicked(false);
    ui->toolButtonNote2->setChecked(false);
    on_toolButtonNote2_clicked(false);
    ui->labelNote1->hide();
    ui->labelNote2->hide();

    if (m_action) {
        m_action->clearCache();
    }

    ui->comboBoxGroup->clear();
    showGroupComboBox(0);
}

void TabOccupancyLiveViewSettings::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_UPDATE_PEOPLECNT_SETTING:
        ON_RESPONSE_FLAG_UPDATE_PEOPLECNT_SETTING(message);
        break;
    case RESPONSE_FLAG_SET_PEOPLECNT_LIVEVIEW_ACTION:
        ON_RESPONSE_FLAG_SET_PEOPLECNT_LIVEVIEW_ACTION(message);
        break;

    }
}

void TabOccupancyLiveViewSettings::ON_RESPONSE_FLAG_UPDATE_PEOPLECNT_SETTING(MessageReceive *message)
{
    Q_UNUSED(message)

    m_eventLoop.exit();
}

void TabOccupancyLiveViewSettings::ON_RESPONSE_FLAG_SET_PEOPLECNT_LIVEVIEW_ACTION(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit();
}

void TabOccupancyLiveViewSettings::showGroupComboBox(int defaultiGroup)
{
    ui->comboBoxGroup->beginEdit();
    ui->comboBoxGroup->clear();
    for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
        const PEOPLECNT_SETTING &setting = m_settings_info.sets[i];
        if (PeopleCountingData::hasChannel(setting.tri_channels)) {
            ui->comboBoxGroup->addItem(QString("%1").arg(i + 1), i);
        }
    }
    ui->comboBoxGroup->endEdit();
    //
    m_currentGroupIndex = -1;
    int index = ui->comboBoxGroup->findData(defaultiGroup);
    if (index < 0) {
        index = 0;
    }
    if (ui->comboBoxGroup->count() == 0) {
        ui->comboBoxGroup->setCurrentIndex(-1);
    } else {
        ui->comboBoxGroup->setCurrentIndex(index);
    }
}

void TabOccupancyLiveViewSettings::cacheGroupSettings(int index)
{
    if (index < 0 || index >= MAX_PEOPLECNT_GROUP) {
        return;
    }

    PEOPLECNT_SETTING &setting = m_settings_info.sets[index];
    setting.enable = ui->comboBoxPeopleCounting->currentData().toInt();
    setting.stays = ui->lineEditMaxStays->text().toInt();
    snprintf(setting.liveview_green_tips, sizeof(setting.liveview_green_tips), "%s", ui->lineEditGreenReminders->text().toStdString().c_str());
    snprintf(setting.liveview_red_tips, sizeof(setting.liveview_red_tips), "%s", ui->lineEditRedReminders->text().toStdString().c_str());
    setting.liveview_font_size = ui->comboBoxFontSize->currentData().toInt();
    setting.liveview_auto_reset = ui->comboBoxLiveViewCountingAutoReset->currentData().toInt();
    setting.auto_day = ui->comboBoxDay->currentData().toInt();
    snprintf(setting.auto_day_time, sizeof(setting.auto_day_time), "%s", ui->timeEditTime->time().toString("HH:mm:ss").toStdString().c_str());
}

void TabOccupancyLiveViewSettings::onLanguageChanged()
{
    ui->labelGroupSettings->setText(GET_TEXT("OCCUPANCY/74216", "Group Settings"));
    ui->pushButtonGroupSettings->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->labelGroup->setText(GET_TEXT("OCCUPANCY/74203", "Group"));
    ui->labelGroupName->setText(GET_TEXT("OCCUPANCY/74217", "Group Name"));
    ui->labelPeopleCounting->setText(GET_TEXT("SMARTEVENT/55008", "People Counting"));

    ui->labelMaxStays->setText(GET_TEXT("OCCUPANCY/74218", "Max. Stay"));
    ui->labelGreenReminders->setText(GET_TEXT("OCCUPANCY/74232", "Reminders of Green Light"));
    ui->labelRedReminders->setText(GET_TEXT("OCCUPANCY/74233", "Reminders of Red Light"));
    ui->labelFontSize->setText(GET_TEXT("OCCUPANCY/74228", "Font Size"));
    ui->labelLiveViewCountingReset->setText(GET_TEXT("OCCUPANCY/74224", "Live View Counting Reset"));
    ui->pushButtonLiveViewCountingReset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->labelLiveViewCountingAutoReset->setText(GET_TEXT("OCCUPANCY/74225", "Live View Counting Auto Reset"));
    ui->labelDay->setText(GET_TEXT("OCCUPANCY/74226", "Day"));
    ui->labelTime->setText(GET_TEXT("OCCUPANCY/74227", "Time"));
    ui->labelNvrCountingReset->setText(GET_TEXT("OCCUPANCY/74219", "NVR Counting Reset"));
    ui->pushButtonNvrCountingReset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->labelAlarmAction->setText(GET_TEXT("OCCUPANCY/74252", "Alarm Action"));
    ui->pushButtonAlarmAction->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->labelNote1->setText(GET_TEXT("OCCUPANCY/74237", "Reset the group counting data in live view."));
    ui->labelNote2->setText(GET_TEXT("OCCUPANCY/74236", "Reset the group counting data stored in NVR side."));

    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabOccupancyLiveViewSettings::on_pushButtonGroupSettings_clicked()
{
    return;
    if (!m_groupSettings) {
        m_groupSettings = new OccupancyGroupSettings(this);
    }
    cacheGroupSettings(ui->comboBoxGroup->currentData().toInt());
    m_groupSettings->setSourceData(m_settings_info.sets);
    int result = m_groupSettings->exec();
    if (result == OccupancyGroupSettings::Accepted) {
        showGroupComboBox(ui->comboBoxGroup->currentData().toInt());
    }
}

void TabOccupancyLiveViewSettings::on_comboBoxGroup_indexSet(int index)
{
    if (m_currentGroupIndex >= 0) {
        if (ui->lineEditMaxStays->isEnabled() && !ui->lineEditMaxStays->checkValid()) {
            ui->comboBoxGroup->beginEdit();
            ui->comboBoxGroup->setCurrentIndex(m_currentGroupIndex);
            ui->comboBoxGroup->endEdit();
            return;
        }
    }
    bool enable = index >= 0;
    ui->lineEditMaxStays->setValid(true);
    ui->comboBoxPeopleCounting->setEnabled(enable);
    ui->lineEditMaxStays->setEnabled(enable);
    ui->lineEditGreenReminders->setEnabled(enable);
    ui->lineEditRedReminders->setEnabled(enable);
    ui->comboBoxFontSize->setEnabled(enable);
    ui->pushButtonLiveViewCountingReset->setEnabled(enable);
    ui->toolButtonNote1->setEnabled(enable);
    ui->comboBoxLiveViewCountingAutoReset->setEnabled(enable);
    ui->pushButtonNvrCountingReset->setEnabled(enable);
    ui->toolButtonNote2->setEnabled(enable);
    ui->pushButtonAlarmAction->setEnabled(enable);

    if (1) {
        ui->lineEditGroupName->clear();
        ui->comboBoxPeopleCounting->setCurrentIndexFromData(0);
        ui->lineEditMaxStays->setText("99999");
        ui->lineEditGreenReminders->setText("Welcome!!!");
        ui->lineEditRedReminders->setText("Please wait till the green light turn on.");
        ui->comboBoxFontSize->setCurrentIndexFromData(1);
        ui->comboBoxLiveViewCountingAutoReset->setCurrentIndexFromData(0);
        ui->comboBoxDay->setCurrentIndexFromData(0);
        ui->timeEditTime->setTime(QTime(0, 0, 0));
    } else {
        int group_id = ui->comboBoxGroup->itemData(index).toInt();
        if (m_currentGroupIndex != -1) {
            cacheGroupSettings(m_currentGroupIndex);
        }
        //
        const PEOPLECNT_SETTING &setting = m_settings_info.sets[group_id];
        ui->lineEditGroupName->setText(setting.name);
        ui->comboBoxPeopleCounting->setCurrentIndexFromData(setting.enable);
        ui->lineEditMaxStays->setText(QString("%1").arg(setting.stays));
        ui->lineEditGreenReminders->setText(setting.liveview_green_tips);
        ui->lineEditRedReminders->setText(setting.liveview_red_tips);
        ui->comboBoxFontSize->setCurrentIndexFromData(setting.liveview_font_size);
        ui->comboBoxLiveViewCountingAutoReset->setCurrentIndexFromData(setting.liveview_auto_reset);
        ui->comboBoxDay->setCurrentIndexFromData(setting.auto_day);
        ui->timeEditTime->setTime(QTime::fromString(setting.auto_day_time, "HH:mm:ss"));
        //
        m_currentGroupIndex = group_id;
    }
}

void TabOccupancyLiveViewSettings::on_comboBoxPeopleCounting_indexSet(int index)
{
    bool enable = ui->comboBoxPeopleCounting->itemData(index).toInt();
    ui->lineEditMaxStays->setEnabled(enable);
    ui->lineEditGreenReminders->setEnabled(enable);
    ui->lineEditRedReminders->setEnabled(enable);
    ui->comboBoxFontSize->setEnabled(enable);
    ui->pushButtonLiveViewCountingReset->setEnabled(enable);
    ui->toolButtonNote1->setEnabled(enable);
    ui->comboBoxLiveViewCountingAutoReset->setEnabled(enable);
    if (ui->comboBoxLiveViewCountingAutoReset->currentData().toInt()) {
        ui->comboBoxDay->setEnabled(enable);
        ui->timeEditTime->setEnabled(enable);
    }
    ui->pushButtonNvrCountingReset->setEnabled(enable);
    ui->toolButtonNote2->setEnabled(enable);
    ui->pushButtonAlarmAction->setEnabled(enable);
}

void TabOccupancyLiveViewSettings::on_pushButtonLiveViewCountingReset_clicked()
{
    ui->pushButtonLiveViewCountingReset->clearUnderMouse();

    int result = ExecQuestionBox(GET_TEXT("OCCUPANCY/74235", "Do you really want to reset the group counting data in live view?"));
    if (result == MessageBox::Yes) {
        int group = ui->comboBoxGroup->currentData().toInt();
        if (LiveViewOccupancyManager::instance()) {
            LiveViewOccupancyManager::instance()->sendLiveViewReset(group);
        }
    }
}

void TabOccupancyLiveViewSettings::on_pushButtonNvrCountingReset_clicked()
{
    ui->pushButtonNvrCountingReset->clearUnderMouse();

    int result = ExecQuestionBox(GET_TEXT("OCCUPANCY/74234", "Do you really want to reset the group counting data stored in NVR side?"));
    if (result == MessageBox::Yes) {
        int group = ui->comboBoxGroup->currentData().toInt();
        if (LiveViewOccupancyManager::instance()) {
            LiveViewOccupancyManager::instance()->sendDatabaseReset(group);
        }
    }
}

void TabOccupancyLiveViewSettings::on_comboBoxLiveViewCountingAutoReset_indexSet(int index)
{
    bool settingEnable = ui->comboBoxPeopleCounting->currentData().toInt();
    bool resetEnable = ui->comboBoxLiveViewCountingAutoReset->itemData(index).toInt();
    ui->comboBoxDay->setEnabled(settingEnable && resetEnable);
    ui->timeEditTime->setEnabled(settingEnable && resetEnable);
}

void TabOccupancyLiveViewSettings::on_pushButtonAlarmAction_clicked()
{
    if (!m_action) {
        m_action = new ActionOccupancy(this);
    }
    m_action->showAction(m_currentGroupIndex);
}

void TabOccupancyLiveViewSettings::on_toolButtonNote1_clicked(bool checked)
{
    if (checked) {
        ui->toolButtonNote1->setIcon(QIcon(":/common/common/note_hover.png"));
    } else {
        ui->toolButtonNote1->setIcon(QIcon(":/common/common/note.png"));
    }

    ui->labelNote1->setVisible(checked);
}

void TabOccupancyLiveViewSettings::on_toolButtonNote2_clicked(bool checked)
{
    if (checked) {
        ui->toolButtonNote2->setIcon(QIcon(":/common/common/note_hover.png"));
    } else {
        ui->toolButtonNote2->setIcon(QIcon(":/common/common/note.png"));
    }

    ui->labelNote2->setVisible(checked);
}

void TabOccupancyLiveViewSettings::on_pushButtonApply_clicked()
{
    return;
    if (ui->lineEditMaxStays->isEnabled() && !ui->lineEditMaxStays->checkValid()) {
        return;
    }
    if (ui->comboBoxGroup->currentIndex() > -1) {
        cacheGroupSettings(ui->comboBoxGroup->currentData().toInt());
    }

    //
    write_peoplecnt_settings(SQLITE_FILE_NAME, m_settings_info.sets, MAX_PEOPLECNT_GROUP);

    //showWait();
    if (m_action) {
        m_action->saveAction();
    }
    struct peoplecnt_action info;
    info.groupid = m_currentGroupIndex;
    info.actmask = ~0;
    Q_UNUSED(info)


    //closeWait();

    //更新预览
    LiveViewOccupancyManager::instance()->updateData();
}

void TabOccupancyLiveViewSettings::on_pushButtonBack_clicked()
{
    back();
}
