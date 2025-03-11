#include "TabPtzWiper.h"
#include "ui_TabPtzWiper.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"

TabPtzWiper::TabPtzWiper(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::TabPtzWiper)
{
    ui->setupUi(this);
    //channel
    ui->comboBoxChannel->clear();
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        ui->comboBoxChannel->addItem(QString::number(i + 1), i);
    }
    ui->comboBoxAutoWiper->clear();
    ui->comboBoxAutoWiper->addItem(GET_TEXT("COMMON/1012", "On"), 1);
    ui->comboBoxAutoWiper->addItem(GET_TEXT("COMMON/1013", "Off"), 0);

    onLanguageChanged();
}

TabPtzWiper::~TabPtzWiper()
{
    delete ui;
}

void TabPtzWiper::initializeData()
{
    ui->comboBoxChannel->setCurrentIndex(currentChannel());
    on_comboBoxChannel_activated(ui->comboBoxChannel->currentIndex());
}

void TabPtzWiper::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_PTZ_WIPER:
        ON_RESPONSE_FLAG_GET_IPC_PTZ_WIPER(message);
        break;
    case RESPONSE_FLAG_SET_IPC_PTZ_WIPER:
        ON_RESPONSE_FLAG_SET_IPC_PTZ_WIPER(message);
        break;
    }
}

void TabPtzWiper::ON_RESPONSE_FLAG_GET_IPC_PTZ_WIPER(MessageReceive *message)
{
    IpcPtzWiper *wiper = static_cast<IpcPtzWiper *>(message->data);
    if (!wiper || !wiper->wiperSupport) {
        m_eventLoop.exit(-1);
        return;
    }
    ui->comboBoxAutoWiper->setCurrentIndexFromData(wiper->autoWiper);
    m_eventLoop.exit(0);
}

void TabPtzWiper::ON_RESPONSE_FLAG_SET_IPC_PTZ_WIPER(MessageReceive *message)
{
    Q_UNUSED(message);
    //closeWait();
}

void TabPtzWiper::setWpierEnable(bool enable)
{
    ui->comboBoxAutoWiper->setEnabled(enable);
    ui->pushButtonCopy->setEnabled(enable);
    ui->pushButtonApply->setEnabled(enable);
    ui->widgetMessage->setVisible(!enable);
}

void TabPtzWiper::onLanguageChanged()
{
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelAutoWiper->setText(GET_TEXT("PTZCONFIG/166034", "Auto Wiper"));

    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonCopy->setText(GET_TEXT("COMMON/1005", "Copy"));
}

void TabPtzWiper::on_comboBoxChannel_activated(int index)
{
    m_channel = ui->comboBoxChannel->itemData(index).toInt();
    setCurrentChannel(m_channel);
    ui->comboBoxAutoWiper->setCurrentIndex(1);
    do {
        if (!LiveView::instance()->isChannelConnected(m_channel)) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
            break;
        }

        //showWait();

        sendMessage(REQUEST_FLAG_GET_IPC_PTZ_WIPER, &m_channel, sizeof(int));
        // int result = m_eventLoop.exec();
        // if (result != 0) {
        //     ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        //     break;
        // }

        //
        setWpierEnable(true);
        //closeWait();
        return;
    } while (0);
    setWpierEnable(false);
    //closeWait();
}

void TabPtzWiper::on_pushButtonCopy_clicked()
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

void TabPtzWiper::on_pushButtonApply_clicked()
{
    //showWait();
    IpcPtzWiper wiper;
    memset(&wiper, 0, sizeof(IpcPtzWiper));
    wiper.autoWiper = ui->comboBoxAutoWiper->currentIntData();
    if (m_copyList.isEmpty()) {
        wiper.chnId = m_channel;
    } else {
        wiper.chnId = -1;
        for (int i = 0; i < m_copyList.size(); ++i) {
            int c = m_copyList.at(i);
            wiper.copyChn[c] = '1';
        }
    }
    sendMessage(REQUEST_FLAG_SET_IPC_PTZ_WIPER, &wiper, sizeof(IpcPtzWiper));
    //显示等待，避免频繁点击
    // QEventLoop eventLoop;
    // QTimer::singleShot(500, &eventLoop, SLOT(quit()));
    // eventLoop.exec();
}

void TabPtzWiper::on_pushButtonBack_clicked()
{
    back();
}
