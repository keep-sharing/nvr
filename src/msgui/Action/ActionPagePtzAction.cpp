#include "ActionPagePtzAction.h"
#include "ui_ActionPagePtzAction.h"
#include "ActionAbstract.h"
#include "AddPtzAction.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "MyButtonGroup.h"
#include "MyDebug.h"

ActionPagePtzAction::ActionPagePtzAction(QWidget *parent)
    : ActionPageAbstract(parent)
    , ui(new Ui::ActionPagePtzAction)
{
    ui->setupUi(this);

    m_group = new MyButtonGroup(this);
    m_group->addButton(ui->pushButtonPtz, m_action->scheduleType());
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

    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("ALARMIN/52014", "Action Type");
    headerList << GET_TEXT("CAMERASEARCH/32002", "No.");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableViewPtzAction->setHorizontalHeaderLabels(headerList);
    ui->tableViewPtzAction->setHeaderCheckable(false);
    ui->tableViewPtzAction->setColumnCount(headerList.size());
    ui->tableViewPtzAction->setSortingEnabled(false);
    ui->tableViewPtzAction->hideColumn(ColumnCheck);
    ui->tableViewPtzAction->setItemDelegateForColumn(ColumnChannel, new ItemIconDelegate(this));
    ui->tableViewPtzAction->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableViewPtzAction->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    ui->tableViewPtzAction->setColumnWidth(ColumnCheck, 0);
    ui->tableViewPtzAction->setColumnWidth(ColumnChannel, 160);
    ui->tableViewPtzAction->setColumnWidth(ColumnActionType, 160);
    ui->tableViewPtzAction->setColumnWidth(ColumnNo, 150);
    ui->tableViewPtzAction->setColumnWidth(ColumnEdit, 150);
    ui->tableViewPtzAction->setColumnWidth(ColumnDelete, 150);
    connect(ui->tableViewPtzAction, SIGNAL(itemClicked(int, int)), this, SLOT(onTableViewPtzActionClicked(int, int)));

    m_addAction = new AddPtzAction(this);
}

ActionPagePtzAction::~ActionPagePtzAction()
{
    delete ui;
}

int ActionPagePtzAction::loadData()
{
    if (hasCache()) {
    } else {
        //
        m_group->setCurrentButton(ui->pushButtonPtz);
        ui->scheduleWidget->setSchedule(m_action->ptzActionSchedule());

        //
        ui->comboBoxTriggerInterval->setCurrentIndexFromData(*m_action->ptzActionTriggerInterval());
        ui->labelPtzAction->setColor(m_action->actionColor());
        //
        m_ptzActionMap.clear();
        int *ptzActionsCount = m_action->ptzActionsCount();
        ptz_action_params *ptzActions = m_action->ptzActions();
        for (int i = 0; i < *ptzActionsCount; ++i) {
            const ptz_action_params &action = ptzActions[i];
            //底层定义的acto_ptz_channel第一个通道是1
            if (action.acto_ptz_channel <= 0) {
                continue;
            }
            m_ptzActionMap.insert(action.acto_ptz_channel, action);
        }
        updatePtzActionTable();

        //
        setCached();
    }
    return 0;
}

int ActionPagePtzAction::saveData()
{
    ui->scheduleWidget->getSchedule(m_action->ptzActionSchedule());

    //
    int *triggerInterval = m_action->ptzActionTriggerInterval();
    *triggerInterval = ui->comboBoxTriggerInterval->currentData().toInt();
    //
    ptz_action_params *ptzActions = m_action->ptzActions();
    int ptzActionsArraySize = m_action->ptzActionsArraySize();
    memset(ptzActions, 0, sizeof(ptz_action_params) * ptzActionsArraySize);
    int i = 0;
    for (auto iter = m_ptzActionMap.constBegin(); iter != m_ptzActionMap.constEnd(); ++iter) {
        const ptz_action_params &param = iter.value();
        memcpy(&ptzActions[i], &param, sizeof(ptz_action_params));
        i++;
    }
    *m_action->ptzActionsCount() = i;
    return 0;
}

void ActionPagePtzAction::updatePtzActionTable()
{
    ui->tableViewPtzAction->clearContent();
    ui->tableViewPtzAction->setRowCount(m_ptzActionMap.size() + 1);
    int row = 0;
    for (auto iter = m_ptzActionMap.constBegin(); iter != m_ptzActionMap.constEnd(); ++iter) {
        const ptz_action_params &action = iter.value();

        ui->tableViewPtzAction->setItemIntValue(row, ColumnCheck, false);

        ui->tableViewPtzAction->setItemIntValue(row, ColumnChannel, action.acto_ptz_channel);

        if (action.acto_ptz_type == PRESET) {
            ui->tableViewPtzAction->setItemText(row, ColumnActionType, GET_TEXT("PTZDIALOG/21015", "Preset"));
            ui->tableViewPtzAction->setItemIntValue(row, ColumnNo, action.acto_ptz_preset);
        } else if (action.acto_ptz_type == PATROL) {
            ui->tableViewPtzAction->setItemText(row, ColumnActionType, GET_TEXT("PTZDIALOG/21016", "Patrol"));
            ui->tableViewPtzAction->setItemIntValue(row, ColumnNo, action.acto_ptz_patrol);
        }

        static QPixmap pixmap(":/common/common/edit.png");
        ui->tableViewPtzAction->setItemPixmap(row, ColumnEdit, pixmap);

        static QPixmap pixmap1(":/common/common/delete.png");
        ui->tableViewPtzAction->setItemPixmap(row, ColumnDelete, pixmap1);

        row++;
    }

    static QPixmap pixmap(":/ptz/ptz/add.png");
    ui->tableViewPtzAction->setItemPixmap(row, ColumnChannel, pixmap);
    ui->tableViewPtzAction->setItemText(row, ColumnActionType, "-");
    ui->tableViewPtzAction->setItemText(row, ColumnNo, "-");
    ui->tableViewPtzAction->setItemText(row, ColumnEdit, "-");
    ui->tableViewPtzAction->setItemText(row, ColumnDelete, "-");

    ui->tableViewPtzAction->scrollToBottom();
}

void ActionPagePtzAction::onLanguageChanged()
{
    ui->pushButtonPtz->setText(GET_TEXT("LIVEVIEW/20021", "PTZ"));
    ui->pushButtonErase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
    ui->labelTriggerInterval->setText(GET_TEXT("VIDEOLOSS/50006", "Triggered Interval"));
    ui->groupBoxPtzAction->setTitle(GET_TEXT("VIDEOLOSS/50014", "PTZ Action"));
}

void ActionPagePtzAction::onScheduleTypeClicked(int id)
{
    ui->scheduleWidget->setCurrentType(id);
}

void ActionPagePtzAction::onTableViewPtzActionClicked(int row, int column)
{
    int channel = m_action->channel();
    int eventType = m_action->eventType();

    int itemChannel = ui->tableViewPtzAction->itemIntValue(row, ColumnChannel);

    ptz_action_params param;
    memset(&param, 0, sizeof(ptz_action_params));

    if (row == ui->tableViewPtzAction->rowCount() - 1) {
        if (column == ColumnChannel) {
            m_addAction->showAdd(channel, eventType, &m_ptzActionMap);
            const int result = m_addAction->exec();
            if (result == AddPtzAction::Accepted) {
                updatePtzActionTable();
            }
        }
    } else {
        if (column == ColumnEdit) {
            const ptz_action_params &action = m_ptzActionMap.value(itemChannel);
            m_addAction->showEdit(channel, eventType, action, &m_ptzActionMap);
            const int result = m_addAction->exec();
            if (result == AddPtzAction::Accepted) {
                updatePtzActionTable();
            }
        } else if (column == ColumnDelete) {
            m_ptzActionMap.remove(itemChannel);
            updatePtzActionTable();
        }
    }
}
