#include "TabNvrAlarmInput.h"
#include "ActionNvrAlarmInput.h"
#include "EffectiveTimeNvrAlarmInput.h"
#include "EventLoop.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "ui_TabNvrAlarmInput.h"
#include <QtDebug>

TabNvrAlarmInput::TabNvrAlarmInput(QWidget *parent) :
    AbstractSettingTab(parent),
    ui(new Ui::TabNvrAlarmInput)
{
    ui->setupUi(this);

    int alarmInput = qMsNvr->maxAlarmInput();
    ui->buttonGroup_alarm->setCount(alarmInput);
    if (alarmInput > 7) {
        ui->horizontalLayout->removeItem(ui->horizontalSpacer);
    }
    connect(ui->buttonGroup_alarm, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroupClicked(int)));

    ui->comboBox_alarmType->clear();
    ui->comboBox_alarmType->addItem(GET_TEXT("ALARMIN/52005", "NO"), 0);
    ui->comboBox_alarmType->addItem(GET_TEXT("ALARMIN/52006", "NC"), 1);

    ui->comboBoxLinkAction->clear();
    ui->comboBoxLinkAction->addItem(GET_TEXT("VIDEOLOSS/50021", "Alarm"), 0);
    ui->comboBoxLinkAction->addItem(GET_TEXT("ALARMIN/175000", "Disarming"), 1);

    QStringList typeList;
    typeList << GET_TEXT("VIDEOLOSS/50009", "Audible Warning") 
             << GET_TEXT("VIDEOLOSS/50010", "Email Linkage")
             << GET_TEXT("SYSTEMGENERAL/70041", "Event Popup") 
             << GET_TEXT("VIDEOLOSS/50014", "PTZ Action") 
             << GET_TEXT("ALARMOUT/53000", "Alarm Output")
             << GET_TEXT("WHITELED/105000", "White LED") 
             << GET_TEXT("ACTION/153000", "HTTP Notification") 
             << GET_TEXT("MENU/10004", "Record")
             << GET_TEXT("SNAPSHOT/95000", "Snapshot")
             << GET_TEXT("ALARMIN/175002", "App Push");

    ui->widgetCheckBoxGroupAction->setCount(typeList.count(), 7);

    ui->widgetCheckBoxGroupAction->setCheckBoxTest(typeList);
    connect(ui->widgetCheckBoxGroupAction, SIGNAL(checkBoxClicked()), this, SLOT(onActionButtonGroupClicked()));

    m_effective = new EffectiveTimeNvrAlarmInput(this);
    m_action    = new ActionNvrAlarmInput(this);
    onLanguageChanged();
}

TabNvrAlarmInput::~TabNvrAlarmInput()
{
    delete ui;
}

void TabNvrAlarmInput::initializeData()
{
    memset(&m_disarm, 0, sizeof(DISARMING_S));
    ui->widgetCheckBoxGroupAction->clearCheck();
    ui->buttonGroup_alarm->setCurrentIndex(0);
}

void TabNvrAlarmInput::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_ALARMIN:
        ON_RESPONSE_FLAG_SET_ALARMIN(message);
        break;
    default:
        break;
    }
}

void TabNvrAlarmInput::ON_RESPONSE_FLAG_SET_ALARMIN(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void TabNvrAlarmInput::saveCurrentAlarmin()
{
    snprintf(m_alarmin.name, sizeof(m_alarmin.name), "%s", ui->lineEdit_alarmName->text().toStdString().c_str());
    m_alarmin.type = ui->comboBox_alarmType->currentData().toInt();
    write_alarm_in(SQLITE_FILE_NAME, &m_alarmin);
    if (m_currentIndex == 0) {
        DISARMING_S temp_disarm;
        memset(&temp_disarm, 0, sizeof(DISARMING_S));
        read_disarming(SQLITE_FILE_NAME, &temp_disarm);
        m_disarm.status = temp_disarm.status;
        write_disarming(SQLITE_FILE_NAME, &m_disarm);
    }

    sendMessage(REQUEST_FLAG_SET_ALARMIN, &m_currentIndex, sizeof(int));
    gEventLoopExec();
}

void TabNvrAlarmInput::onLanguageChanged()
{
    ui->label_alarmInput->setText(GET_TEXT("ALARMIN/52002", "Alarm Input No."));
    ui->label_alarmName->setText(GET_TEXT("ALARMIN/52003", "Alarm Name"));
    ui->label_alarmType->setText(GET_TEXT("ALARMIN/52004", "Alarm Type"));
    ui->label_note->setText(GET_TEXT("ALARMIN/52010", "Note:Alarm Name will not be copied into other channels."));
    ui->label_action->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->labelAction2->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->label_effective->setText(GET_TEXT("ALARMOUT/53003", "Effective Time"));
    ui->pushButton_actionEdit->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_effectiveEdit->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->labelLinkAction->setText(GET_TEXT("ALARMIN/175001", "Linkage Action"));
}

void TabNvrAlarmInput::onButtonGroupClicked(int index)
{
    ui->lineEdit_alarmName->setValid(true);
    if (m_action) {
        m_action->clearCache();
    }
    if (m_effective) {
        m_effective->clearCache();
    }
    m_currentIndex = index;

    memset(&m_alarmin, 0, sizeof(alarm_in));
    read_alarm_in(SQLITE_FILE_NAME, &m_alarmin, m_currentIndex);

    qDebug() << QString("AlarmInput::onButtonGroupClicked, name: %1, type: %2").arg(m_alarmin.name).arg(m_alarmin.type);
    //
    ui->lineEdit_alarmName->setText(QString(m_alarmin.name));
    ui->comboBox_alarmType->setCurrentIndexFromData(m_alarmin.type);

    if (index == 0) {
        read_disarming(SQLITE_FILE_NAME, &m_disarm);
        ui->comboBoxLinkAction->setCurrentIndexFromData(m_disarm.linkageAction);
        ui->widgetCheckBoxGroupAction->setCheckedFromInt(m_disarm.disarmActionMask);
        ui->comboBoxLinkAction->show();
        ui->labelLinkAction->show();
    } else {
        ui->comboBoxLinkAction->setCurrentIndexFromData(0);
        ui->comboBoxLinkAction->hide();
        ui->labelLinkAction->hide();
    }
}

void TabNvrAlarmInput::on_pushButton_actionEdit_clicked()
{
    m_action->showAction(m_currentIndex);

    read_alarm_in(SQLITE_FILE_NAME, &m_alarmin, m_currentIndex);

    ui->pushButton_actionEdit->clearUnderMouse();
}

void TabNvrAlarmInput::on_pushButton_effectiveEdit_clicked()
{
    ui->pushButton_effectiveEdit->clearUnderMouse();

    m_effective->showEffectiveTime(-1, m_currentIndex);
}

void TabNvrAlarmInput::on_pushButton_copy_clicked()
{
    ChannelCopyDialog copy(this);
    copy.setTitle(GET_TEXT("ALARMIN/52007", "Alarm Input Copy"));
    copy.setCount(qMsNvr->maxAlarmInput());
    copy.setCurrentChannel(m_currentIndex);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        QList<int> copyList  = copy.checkedList(false);
        quint64    copyFlags = copy.checkedFlags(false);

        //showWait();
        // 先保存当前通道
        m_effective->saveEffectiveTime();
        m_action->saveAction();
        read_alarm_in(SQLITE_FILE_NAME, &m_alarmin, m_currentIndex);
        saveCurrentAlarmin();

        // copy effective time and action
        smart_event_schedule *currentEffectiveTimeSchedule1 = new smart_event_schedule;
        memset(currentEffectiveTimeSchedule1, 0, sizeof(smart_event_schedule));
        read_alarmin_effective_schedule(SQLITE_FILE_NAME, currentEffectiveTimeSchedule1, m_currentIndex);
        copy_alarmin_effective_schedules(SQLITE_FILE_NAME, currentEffectiveTimeSchedule1, copyFlags);
        delete currentEffectiveTimeSchedule1;
        //
        alarm_in *currentAlarmin = new alarm_in;
        memset(currentAlarmin, 0, sizeof(alarm_in));
        read_alarm_in(SQLITE_FILE_NAME, currentAlarmin, m_currentIndex);
        copy_alarm_in_events(SQLITE_FILE_NAME, currentAlarmin, MAX_ALARM_IN, copyFlags);
        delete currentAlarmin;
        //
        alarm_in_schedule *currentActionAudible = new alarm_in_schedule;
        memset(currentActionAudible, 0, sizeof(alarm_in_schedule));
        read_alarmin_audible_schedule(SQLITE_FILE_NAME, currentActionAudible, m_currentIndex);
        copy_alarmin_audible_schedules(SQLITE_FILE_NAME, currentActionAudible, copyFlags);
        delete currentActionAudible;
        //
        alarm_in_schedule *currentActionEmail = new alarm_in_schedule;
        memset(currentActionEmail, 0, sizeof(alarm_in_schedule));
        read_alarmin_email_schedule(SQLITE_FILE_NAME, currentActionEmail, m_currentIndex);
        copy_alarmin_email_schedules(SQLITE_FILE_NAME, currentActionEmail, copyFlags);
        delete currentActionEmail;
        // event popup
        alarm_in_schedule *currentActionPopup = new alarm_in_schedule;
        memset(currentActionPopup, 0, sizeof(alarm_in_schedule));
        read_alarmin_popup_schedule(SQLITE_FILE_NAME, currentActionPopup, m_currentIndex);
        copy_alarmin_popup_schedules(SQLITE_FILE_NAME, currentActionPopup, copyFlags);
        delete currentActionPopup;
        // ptz action schedule
        alarm_in_schedule *currentActionPtz = new alarm_in_schedule;
        memset(currentActionPtz, 0, sizeof(alarm_in_schedule));
        read_alarmin_ptz_schedule(SQLITE_FILE_NAME, currentActionPtz, m_currentIndex);
        copy_alarmin_ptz_schedules(SQLITE_FILE_NAME, currentActionPtz, copyFlags);
        delete currentActionPtz;
        // ptz action
        ptz_action_params *ptzActionArray = new ptz_action_params[MAX_CAMERA];
        int                ptzItemCount   = 0;
        memset(ptzActionArray, 0x0, sizeof(ptz_action_params) * MAX_CAMERA);
        read_ptz_params(SQLITE_FILE_NAME, ptzActionArray, ALARMIO, m_currentIndex, &ptzItemCount);
        copy_ptz_params_all(SQLITE_FILE_NAME, ptzActionArray, ALARMIO, copyFlags);
        delete[] ptzActionArray;
        // led schedule
        SMART_SCHEDULE *scheduleLed = new SMART_SCHEDULE;
        memset(scheduleLed, 0, sizeof(SMART_SCHEDULE));
        WLED_INFO led_info_schedule;
        led_info_schedule.chnid = m_currentIndex;
        snprintf(led_info_schedule.pDbTable, sizeof(led_info_schedule.pDbTable), "%s", AIN_WLED_ESCHE);
        read_whiteled_effective_schedule(SQLITE_FILE_NAME, scheduleLed, &led_info_schedule);
        copy_whiteled_effective_schedules(SQLITE_FILE_NAME, scheduleLed, &led_info_schedule, copyFlags);
        delete scheduleLed;
        // led params
        WHITE_LED_PARAMS *ledParamsArray = new WHITE_LED_PARAMS[MAX_CAMERA];
        memset(ledParamsArray, 0, sizeof(WHITE_LED_PARAMS) * MAX_CAMERA);
        WLED_INFO led_info_params;
        led_info_params.chnid = m_currentIndex;
        snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", AIN_WLED_PARAMS);
        int led_params_count = 0;
        read_whiteled_params(SQLITE_FILE_NAME, ledParamsArray, &led_info_params, &led_params_count);
        copy_whiteled_params(SQLITE_FILE_NAME, AIN_WLED_PARAMS, ledParamsArray, copyFlags);
        delete[] ledParamsArray;

        // http
        copy_http_notification_action(ALARMIO, m_currentIndex, copyFlags);

        //
        for (int i = 0; i < copyList.size(); ++i) {
            int index = copyList.at(i);
            if (index != m_currentIndex) {
                sendMessage(REQUEST_FLAG_SET_ALARMIN, &index, sizeof(int));
                gEventLoopExec();
            }
        }
        //closeWait();
    }
}

void TabNvrAlarmInput::on_pushButton_apply_clicked()
{
    QString strName = ui->lineEdit_alarmName->text().trimmed();
    if (strName.contains("'")) {
        ui->lineEdit_alarmName->setCustomValid(false, GET_TEXT("MYLINETIP/112013", "Invalid characters: '."));
        return;
    }
    //showWait();
    m_effective->saveEffectiveTime();
    m_action->saveAction();
    read_alarm_in(SQLITE_FILE_NAME, &m_alarmin, m_currentIndex);
    saveCurrentAlarmin();
    //closeWait();
}

void TabNvrAlarmInput::on_pushButton_back_clicked()
{
    emit sig_back();
}

void TabNvrAlarmInput::on_comboBoxLinkAction_indexSet(int index)
{
    if (index == 1) {
        ui->widget->hide();
        ui->widget_1->hide();
        ui->label_action->hide();
        ui->label_effective->hide();
        ui->widgetCheckBoxGroupAction->show();
        ui->widgetAction->show();
        ui->pushButton_copy->hide();
    } else {
        ui->widgetCheckBoxGroupAction->hide();
        ui->widgetAction->hide();
        ui->widget->show();
        ui->widget_1->show();
        ui->label_action->show();
        ui->label_effective->show();
        ui->pushButton_copy->show();
    }
    if (m_currentIndex == 0) {
        m_disarm.linkageAction = static_cast<ALARMIN_LINKAGE_ACTION_E>(index);
    }
}

void TabNvrAlarmInput::onActionButtonGroupClicked()
{
    m_disarm.disarmActionMask = ui->widgetCheckBoxGroupAction->checkedFlags();
}