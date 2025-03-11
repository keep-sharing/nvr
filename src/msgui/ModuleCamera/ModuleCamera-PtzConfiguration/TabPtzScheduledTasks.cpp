#include "TabPtzScheduledTasks.h"
#include "ui_TabPtzScheduledTasks.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"

TabPtzScheduledTasks::TabPtzScheduledTasks(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::TabPtzScheduledTasks)
{
    ui->setupUi(this);
    ui->comboBoxChannel->clear();
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        ui->comboBoxChannel->addItem(QString::number(i + 1), i);
    }

    m_ptzSchedule = new PtzScheduleTask(this);
    QRegExp regx("[0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, ui->lineEditLatencyTime);
    ui->lineEditLatencyTime->setValidator(validator);
    ui->lineEditLatencyTime->setRange(5, 720);

    onLanguageChanged();
}

TabPtzScheduledTasks::~TabPtzScheduledTasks()
{
    delete ui;
}

void TabPtzScheduledTasks::initializeData()
{
    ui->comboBoxChannel->setCurrentIndex(currentChannel());
    on_comboBoxChannel_activated(ui->comboBoxChannel->currentIndex());
}

void TabPtzScheduledTasks::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_PTZ_SCHE_TASK:
        ON_RESPONSE_FLAG_GET_IPC_PTZ_SCHE_TASK(message);
        break;
    case RESPONSE_FLAG_SET_IPC_PTZ_SCHE_TASK:
        ON_RESPONSE_FLAG_SET_IPC_PTZ_SCHE_TASK(message);
        break;
    }
}

void TabPtzScheduledTasks::ON_RESPONSE_FLAG_GET_IPC_PTZ_SCHE_TASK(MessageReceive *message)
{
    IPC_PTZ_SCHE_TASK_INFO_S *scheInfo = static_cast<IPC_PTZ_SCHE_TASK_INFO_S *>(message->data);
    if (!scheInfo || !scheInfo->ptzSupport) {
        m_eventLoop.exit(-1);
        return;
    }
    memset(&m_scheInfo, 0, sizeof(IPC_PTZ_SCHE_TASK_INFO_S));
    memcpy(&m_scheInfo, scheInfo, sizeof(IPC_PTZ_SCHE_TASK_INFO_S));

    m_eventLoop.exit(0);
}

void TabPtzScheduledTasks::ON_RESPONSE_FLAG_SET_IPC_PTZ_SCHE_TASK(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit();
}

void TabPtzScheduledTasks::setSettingEnable(bool enable)
{
    ui->pushButtonCopy->setEnabled(enable);
    ui->pushButtonApply->setEnabled(enable);
    ui->pushButtonSchedule->setEnabled(enable);
    ui->lineEditLatencyTime->setText("5");
    ui->lineEditLatencyTime->setEnabled(enable);
    ui->checkBoxEnable->setChecked(false);
    ui->checkBoxEnable->setEnabled(enable);
    ui->widgetMessage->setVisible(!enable);
}

void TabPtzScheduledTasks::setScheduleArray(IPC_PTZ_SCHE_TASK_WEEK_S &schedule)
{
    m_ptzSchedule->setSchedule(schedule);
}

void TabPtzScheduledTasks::getScheduleArray(IPC_PTZ_SCHE_TASK_WEEK_S &schedule)
{
    m_ptzSchedule->getSchedule(schedule);
}

void TabPtzScheduledTasks::onLanguageChanged()
{
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelSchduleTasks->setText(GET_TEXT("PTZCONFIG/166003","Schedule Tasks"));
    ui->labelScheduleSettings->setText(GET_TEXT("PTZCONFIG/166033", "Schedule Settings"));
    ui->labelLatencyTime->setText(GET_TEXT("PTZCONFIG/166016","Latency Time"));
    ui->pushButtonSchedule->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonCopy->setText(GET_TEXT("COMMON/1005", "Copy"));
}

void TabPtzScheduledTasks::on_comboBoxChannel_activated(int index)
{
    m_channel = ui->comboBoxChannel->itemData(index).toInt();
    setCurrentChannel(m_channel);

    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }

    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPC_PTZ_SCHE_TASK, &m_channel, sizeof(int));
    // int result = m_eventLoop.exec();
    // if (result != 0) {
    //     setSettingEnable(false);
    //     //closeWait();
    //     ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
    //     return;
    // }
    //
    setSettingEnable(true);
    ui->checkBoxEnable->setChecked(m_scheInfo.scheTaskEnable);
    ui->lineEditLatencyTime->setText(QString("%1").arg(m_scheInfo.recoveryTime));
    on_checkBoxEnable_clicked();
    //closeWait();
}

void TabPtzScheduledTasks::on_checkBoxEnable_clicked()
{
    bool enable = ui->checkBoxEnable->isChecked();
    ui->pushButtonSchedule->setEnabled(enable);
    ui->lineEditLatencyTime->setEnabled(enable);
}

void TabPtzScheduledTasks::on_pushButtonSchedule_clicked()
{
    m_ptzSchedule->showAction(m_scheInfo.schedule, m_scheInfo.wiperSupport, m_scheInfo.speedDomeSupport);
    int result = m_ptzSchedule->exec();
    if (result == PtzScheduleTask::Accepted) {
        getScheduleArray(m_scheInfo.schedule);
    }
}

void TabPtzScheduledTasks::on_pushButtonCopy_clicked()
{
    m_copyList.clear();

    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_channel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        m_copyList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(on_pushButtonApply_clicked()));
    }
}

void TabPtzScheduledTasks::on_pushButtonApply_clicked()
{
    if (!ui->lineEditLatencyTime->checkValid()) {
        return;
    }
    //showWait();

    if (m_copyList.isEmpty()) {
        m_scheInfo.chnId = m_channel;
    } else {
        m_scheInfo.chnId = -1;
        for (int i = 0; i < m_copyList.size(); ++i) {
            int c = m_copyList.at(i);
            m_scheInfo.copyChn[c] = '1';
        }
    }
    m_scheInfo.scheTaskEnable = ui->checkBoxEnable->isChecked();
    m_scheInfo.recoveryTime = ui->lineEditLatencyTime->text().toInt();
    sendMessage(REQUEST_FLAG_SET_IPC_PTZ_SCHE_TASK, &m_scheInfo, sizeof(IPC_PTZ_SCHE_TASK_INFO_S));
    //m_eventLoop.exec();
    //closeWait();
}

void TabPtzScheduledTasks::on_pushButtonBack_clicked()
{
    back();
}
