#include "ActionPageWhiteLed.h"
#include "ui_ActionPageWhiteLed.h"
#include "ActionAbstract.h"
#include "AddWhiteLed.h"
#include "AddAlarmOutput.h"
#include "MyButtonGroup.h"
#include "MsLanguage.h"

ActionPageWhiteLed::ActionPageWhiteLed(QWidget *parent)
    : ActionPageAbstract(parent)
    , ui(new Ui::ActionPageWhiteLed)
{
    ui->setupUi(this);

    m_group = new MyButtonGroup(this);
    m_group->addButton(ui->pushButtonWhiteLed, m_action->scheduleType());
    m_group->addButton(ui->pushButtonErase, NONE);
    connect(m_group, SIGNAL(buttonClicked(int)), this, SLOT(onScheduleTypeClicked(int)));

    ui->scheduleWidget->setSingleEditType(m_action->scheduleType());
    if (m_action->eventType() == PEOPLE_COUNT) {
        ui->scheduleWidget->setTypeColor(m_action->scheduleType(), QColor("#FFE793"));
    }

    ui->comboBoxTriggerInterval->clear();
    ui->comboBoxTriggerInterval->addItem("10s", 10);
    ui->comboBoxTriggerInterval->addItem("20s", 20);
    ui->comboBoxTriggerInterval->addItem("40s", 40);
    ui->comboBoxTriggerInterval->addItem("60s", 60);
    ui->comboBoxTriggerInterval->addItem("100s", 100);
    ui->comboBoxTriggerInterval->addItem("5min", 300);
    ui->comboBoxTriggerInterval->addItem("15min", 900);
    ui->comboBoxTriggerInterval->addItem("30min", 1800);
    ui->comboBoxTriggerInterval->addItem("1h", 3600);
    ui->comboBoxTriggerInterval->addItem("8h", 28800);
    ui->comboBoxTriggerInterval->addItem("12h", 43200);
    ui->comboBoxTriggerInterval->addItem("24h", 86400);
    ui->comboBoxTriggerInterval->setCurrentIndexFromData(20);

    QStringList headerLed;
    headerLed << "";
    headerLed << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerLed << GET_TEXT("WHITELED/105001", "Flash Mode");
    headerLed << GET_TEXT("WHITELED/105002", "Flash Time");
    headerLed << GET_TEXT("COMMON/1019", "Edit");
    headerLed << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableViewWhiteLed->setHorizontalHeaderLabels(headerLed);
    ui->tableViewWhiteLed->setHeaderCheckable(false);
    ui->tableViewWhiteLed->setColumnCount(headerLed.size());
    ui->tableViewWhiteLed->setSortingEnabled(false);
    ui->tableViewWhiteLed->hideColumn(LedCheck);
    //
    ui->tableViewWhiteLed->setItemDelegateForColumn(LedChannel, new ItemIconDelegate(this));
    ui->tableViewWhiteLed->setItemDelegateForColumn(LedEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableViewWhiteLed->setItemDelegateForColumn(LedDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //
    ui->tableViewWhiteLed->setColumnWidth(LedChannel, 150);
    ui->tableViewWhiteLed->setColumnWidth(LedMode, 150);
    ui->tableViewWhiteLed->setColumnWidth(LedTime, 150);
    ui->tableViewWhiteLed->setColumnWidth(LedEdit, 150);
    connect(ui->tableViewWhiteLed, SIGNAL(itemClicked(int, int)), this, SLOT(onTableViewWhiteLedClicked(int, int)));

    m_addAction = new AddWhiteLed(this);
}

ActionPageWhiteLed::~ActionPageWhiteLed()
{
    delete ui;
}

void ActionPageWhiteLed::dealMessage(MessageReceive *message)
{
    m_addAction->dealMessage(message);
}

int ActionPageWhiteLed::loadData()
{
    if (hasCache()) {
    } else {
        //
        m_group->setCurrentButton(ui->pushButtonWhiteLed);
        ui->scheduleWidget->setSchedule(m_action->whiteLedSchedule());

        //
        ui->comboBoxTriggerInterval->setCurrentIndexFromData(*m_action->whiteLedTriggerInterval());
        ui->labelWhiteLed->setColor(m_action->actionColor());
        //
        m_whiteLedParamsMap.clear();
        white_led_params *whiteLedParams = m_action->whiteLedParams();
        int *whiteLedParamsCount = m_action->whiteLedParamsCount();
        for (int i = 0; i < *whiteLedParamsCount; ++i) {
            const white_led_params &params = whiteLedParams[i];
            m_whiteLedParamsMap.insert(params.acto_chn_id, params);
        }
        updateWhiteLedTable();

        //
        setCached();
    }
    return 0;
}

int ActionPageWhiteLed::saveData()
{
    ui->scheduleWidget->getSchedule(m_action->whiteLedSchedule());
    //
    int *triggerInterval = m_action->whiteLedTriggerInterval();
    *triggerInterval = ui->comboBoxTriggerInterval->currentData().toInt();
    //
    white_led_params *whiteLedParams = m_action->whiteLedParams();
    int whiteLedParamsArraySize = m_action->whiteLedParamsArraySize();
    memset(whiteLedParams, 0, sizeof(white_led_params) * whiteLedParamsArraySize);
    int i = 0;
    for (auto iter = m_whiteLedParamsMap.constBegin(); iter != m_whiteLedParamsMap.constEnd(); ++iter) {
        const white_led_params &param = iter.value();
        memcpy(&whiteLedParams[i], &param, sizeof(white_led_params));
        i++;
    }
    *m_action->whiteLedParamsCount() = i;
    return 0;
}

void ActionPageWhiteLed::updateWhiteLedTable()
{
    ui->tableViewWhiteLed->clearContent();
    ui->tableViewWhiteLed->setRowCount(m_whiteLedParamsMap.size() + 1);

    int row = 0;
    for (auto iter = m_whiteLedParamsMap.constBegin(); iter != m_whiteLedParamsMap.constEnd(); ++iter) {
        const white_led_params &params = iter.value();
        ui->tableViewWhiteLed->setItemIntValue(row, LedChannel, params.acto_chn_id + 1);
        QString strFlashMode;
        switch (params.flash_mode) {
        case 0:
            strFlashMode = GET_TEXT("ACTION/56001", "Twinkle");
            break;
        case 1:
            strFlashMode = GET_TEXT("ACTION/56002", "Always");
            break;
        }
        ui->tableViewWhiteLed->setItemText(row, LedMode, strFlashMode);
        ui->tableViewWhiteLed->setItemIntValue(row, LedTime, params.flash_time);
        row++;
    }

    ui->tableViewWhiteLed->setItemPixmap(row, LedChannel, QPixmap(":/ptz/ptz/add.png"));
    ui->tableViewWhiteLed->setItemText(row, LedMode, "-");
    ui->tableViewWhiteLed->setItemText(row, LedTime, "-");
    ui->tableViewWhiteLed->setItemText(row, LedEdit, "-");
    ui->tableViewWhiteLed->setItemText(row, LedDelete, "-");

    ui->tableViewWhiteLed->scrollToBottom();
}

void ActionPageWhiteLed::onLanguageChanged()
{
    ui->pushButtonWhiteLed->setText(GET_TEXT("WHITELED/105000", "White LED"));
    ui->pushButtonErase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
    ui->labelTriggerInterval->setText(GET_TEXT("VIDEOLOSS/50006", "Triggered Interval"));
    ui->groupBoxWhiteLed->setTitle(GET_TEXT("WHITELED/105000", "White LED"));
}

void ActionPageWhiteLed::onScheduleTypeClicked(int id)
{
    ui->scheduleWidget->setCurrentType(id);
}

void ActionPageWhiteLed::onTableViewWhiteLedClicked(int row, int column)
{
    int channel = m_action->channel();
    int rowCount = ui->tableViewWhiteLed->rowCount() - 1;
    int acto_chn_id = ui->tableViewWhiteLed->itemIntValue(row, LedChannel) - 1;

    switch (column) {
    case LedChannel: {
        if (row == rowCount) {
             m_addAction->setParamsMap(m_whiteLedParamsMap);
            int result = m_addAction->execAdd();
            if (result == AddWhiteLed::Accepted) {
                WHITE_LED_PARAMS params = m_addAction->params();
                params.chnid = channel;
                m_whiteLedParamsMap.insert(params.acto_chn_id, params);
                updateWhiteLedTable();
            }
        }
        break;
    }
    case LedEdit: {
        if (row < rowCount) {
            const WHITE_LED_PARAMS &params = m_whiteLedParamsMap.value(acto_chn_id);
            m_addAction->setParamsMap(m_whiteLedParamsMap);
            int result = m_addAction->execEdit(params);
            if (result == AddWhiteLed::Accepted) {
                WHITE_LED_PARAMS params = m_addAction->params();
                params.chnid = channel;
                m_whiteLedParamsMap.remove(acto_chn_id);
                m_whiteLedParamsMap.insert(params.acto_chn_id, params);
                updateWhiteLedTable();
            }
        }
        break;
    }
    case LedDelete: {
        if (row < rowCount) {
            m_whiteLedParamsMap.remove(acto_chn_id);
            updateWhiteLedTable();
        }
        break;
    }
    default:
        break;
    }
}
