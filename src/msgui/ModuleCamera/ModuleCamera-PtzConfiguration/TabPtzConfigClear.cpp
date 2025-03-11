#include "TabPtzConfigClear.h"
#include "ui_TabPtzConfigClear.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"

TabPtzConfigClear::TabPtzConfigClear(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::TabPtzConfigClear)
{
    ui->setupUi(this);
    ui->comboBoxChannel->clear();
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        ui->comboBoxChannel->addItem(QString::number(i + 1), i);
    }

    ui->widgetCheckGroup->setCount(9, 3);
    QStringList typeList;
    typeList << GET_TEXT("PTZCONFIG/166019", "All Presets")
             << GET_TEXT("PTZCONFIG/166020", "All Partrols")
             << GET_TEXT("PTZCONFIG/166021", "All Patterns")
             << GET_TEXT("PTZCONFIG/166022", "All Auto Homes")
             << GET_TEXT("PTZCONFIG/166023", "All PTZ Limits")
             << GET_TEXT("PTZCONFIG/166024", "All Scheduled Tasks")
             << GET_TEXT("PTZCONFIG/166025", "All Privacy Masks")
             << GET_TEXT("PTZCONFIG/166026", "Initial Position")
             << GET_TEXT("PTZCONFIG/166027", "Auto Tracking");
    ui->widgetCheckGroup->setCheckBoxTest(typeList);

    onLanguageChanged();
}

TabPtzConfigClear::~TabPtzConfigClear()
{
    delete ui;
}

void TabPtzConfigClear::initializeData()
{
    ui->comboBoxChannel->setCurrentIndex(currentChannel());
    on_comboBoxChannel_activated(ui->comboBoxChannel->currentIndex());
}

void TabPtzConfigClear::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabPtzConfigClear::setSettingEnable(bool enable)
{
    ui->pushButtonApply->setEnabled(enable);
    ui->pushButtonCopy->setEnabled(enable);
    ui->widgetCheckGroup->setEnabled(enable);
}

void TabPtzConfigClear::onLanguageChanged()
{
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelConfigClear->setText(GET_TEXT("PTZCONFIG/166028", "Config Clear"));

    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonCopy->setText(GET_TEXT("COMMON/1005", "Copy"));
}

void TabPtzConfigClear::on_comboBoxChannel_activated(int index)
{
    m_channel = ui->comboBoxChannel->itemData(index).toInt();
    setCurrentChannel(m_channel);
    ui->widgetCheckGroup->clearCheck();
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }
    struct ipc_system_info system_info;
    memset(&system_info, 0, sizeof(system_info));
    //get_ipc_system_info(m_channel, &system_info);
    if (!system_info.ptzSupport) {
        //closeWait();
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }
    ui->widgetMessage->hide();
    setSettingEnable(true);
}

void TabPtzConfigClear::on_pushButtonCopy_clicked()
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

void TabPtzConfigClear::on_pushButtonApply_clicked()
{
    //showWait();
    IpcPtzConfigClear configClear;
    memset(&configClear, 0, sizeof(IpcPtzConfigClear));
    if (m_copyList.isEmpty()) {
        configClear.chnId = m_channel;
    } else {
        configClear.chnId = -1;
        for (int i = 0; i < m_copyList.size(); ++i) {
            int c = m_copyList.at(i);
            configClear.copyChn[c] = '1';
        }
    }
    configClear.clearMask = static_cast<int>(ui->widgetCheckGroup->checkedFlags());
    sendMessage(REQUEST_FLAG_SET_IPC_PTZ_CONFIG_CLEAR, &configClear, sizeof(IpcPtzConfigClear));
    //显示等待，避免频繁点击
    QEventLoop eventLoop;
    QTimer::singleShot(500, &eventLoop, SLOT(quit()));
    eventLoop.exec();
    //closeWait();
}

void TabPtzConfigClear::on_pushButtonBack_clicked()
{
    back();
}
