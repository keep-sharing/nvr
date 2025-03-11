#include "PlaybackMode.h"
#include "ui_PlaybackMode.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "PlaybackBar.h"
#include "PlaybackEventData.h"
#include "PlaybackTagData.h"
#include "PlaybackWindow.h"
#include "SmartSpeedPanel.h"
#include "centralmessage.h"
#include <QtDebug>

PlaybackMode::PlaybackMode(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::PlaybackMode)
{
    ui->setupUi(this);
    s_playbackMode = this;

    ui->toolButton_close->setPixmap(QPixmap(":/mainmenu/mainmenu/preview.png"));
    ui->toolButton_close->setHoverPixmap(QPixmap(":/mainmenu/mainmenu/preview_blue.png"));
    ui->toolButton_close->setPressedPixmap(QPixmap(":/mainmenu/mainmenu/preview.png"));
    connect(ui->toolButton_close, SIGNAL(clicked(bool)), this, SLOT(onToolButtonCloseClicked()));

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PlaybackMode::~PlaybackMode()
{
    s_playbackMode = nullptr;
    delete ui;
}

void PlaybackMode::initializeData()
{
    ui->comboBox_playType->setCurrentIndexFromData(GeneralPlayback);
    on_comboBox_playType_activated(ui->comboBox_playType->currentIndex());

    ui->comboBox_stream->setCurrentIndexFromData(FILE_TYPE_MAIN);
    on_comboBox_stream_activated(ui->comboBox_stream->currentIndex());
}

NetworkResult PlaybackMode::dealNetworkCommond(const QString &commond)
{
    NetworkResult result = NetworkReject;

    switch (playbackType()) {
    case GeneralPlayback:
        if (commond.startsWith("ChangeFocus_")) {
            if (ui->comboBox_playType->hasFocus()) {
                ui->comboBox_stream->setFocus();
            } else if (ui->comboBox_stream->hasFocus() || ui->page_general->hasFocus()) {
                bool ok = ui->page_general->focusNext();
                if (!ok) {
                    ui->toolButton_close->setFocus();
                }
            } else {
                ui->comboBox_playType->setFocus();
            }
            result = NetworkAccept;
        } else if (commond.startsWith("ChangeFocus_Prev")) {
            if (ui->toolButton_close->hasFocus()) {
                bool ok = ui->page_general->focusPrevious();
                if (!ok) {
                    focusPreviousChild();
                }
            }
            result = NetworkAccept;
        } else {
            result = ui->page_general->dealNetworkCommond(commond);
            if (!result) {
                result = s_playbackBar->dealNetworkCommond(commond);
            }
        }
        break;
    case EventPlayback:
        if (commond.startsWith("ChangeFocus_")) {
            if (ui->comboBox_playType->hasFocus()) {
                ui->comboBox_stream->setFocus();
            } else if (ui->comboBox_stream->hasFocus()) {
                ui->page_event->setFocus();
            } else if (ui->page_event->hasFocus()) {
                bool ok = ui->page_event->focusNext();
                if (!ok) {
                    ui->toolButton_close->setFocus();
                }
            } else {
                ui->comboBox_playType->setFocus();
            }
            result = NetworkAccept;
        } else {
            result = ui->page_event->dealNetworkCommond(commond);
            if (result == NetworkReject) {
                result = s_playbackBar->dealNetworkCommond(commond);
            }
        }
        break;
    case TagPlayback:
        if (commond.startsWith("ChangeFocus_")) {
            if (ui->comboBox_playType->hasFocus()) {
                ui->comboBox_stream->setFocus();
            } else if (ui->comboBox_stream->hasFocus()) {
                ui->page_tag->setFocus();
            } else if (ui->page_tag->hasFocus()) {
                bool ok = ui->page_tag->focusNext();
                if (!ok) {
                    ui->toolButton_close->setFocus();
                }
            } else {
                ui->comboBox_playType->setFocus();
            }
            result = NetworkAccept;
        } else {
            result = ui->page_tag->dealNetworkCommond(commond);
            if (result == NetworkReject) {
                result = s_playbackBar->dealNetworkCommond(commond);
            }
        }
        break;
    case SplitPlayback:
        if (commond.startsWith("ChangeFocus_")) {
            if (ui->comboBox_playType->hasFocus()) {
                ui->comboBox_stream->setFocus();
            } else if (ui->comboBox_stream->hasFocus()) {
                ui->page_split->setFocus();
            } else if (ui->page_split->hasFocus()) {
                bool ok = ui->page_split->focusNext();
                if (!ok) {
                    ui->toolButton_close->setFocus();
                }
            } else {
                ui->comboBox_playType->setFocus();
            }
            result = NetworkAccept;
        } else {
            result = ui->page_split->dealNetworkCommond(commond);
            if (result == NetworkReject) {
                result = s_playbackBar->dealNetworkCommond(commond);
            }
        }
        break;
    case PicturePlayback:
        if (commond.startsWith("ChangeFocus_")) {
            if (ui->comboBox_playType->hasFocus()) {
                ui->page_picture->setFocus();
            } else if (ui->page_picture->hasFocus()) {
                bool ok = ui->page_picture->focusNext();
                if (!ok) {
                    ui->toolButton_close->setFocus();
                }
            } else {
                ui->comboBox_playType->setFocus();
            }
            result = NetworkAccept;
        } else {
            result = ui->page_picture->dealNetworkCommond(commond);
        }
        break;
    default:
        break;
    }

    return result;
}

void PlaybackMode::closePlayback()
{
    switch (ui->stackedWidget->currentIndex()) {
    case GeneralPlayback:
        ui->page_general->closePlayback();
        break;
    case EventPlayback:
        ui->page_event->closePlayback();
        break;
    case TagPlayback:
        ui->page_tag->closePlayback();
        break;
    case SplitPlayback:
        ui->page_split->closePlayback();
        break;
    case PicturePlayback:
        ui->page_picture->closePlayback();
        break;
    }
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_CLEAR_TEMP_TAGS_PLAYBACK, NULL, 0);
}

void PlaybackMode::onLanguageChanged()
{
    ui->comboBox_playType->clear();
    ui->comboBox_playType->addItem(GET_TEXT("PLAYBACK/80030", "General Playback"), GeneralPlayback);
    if (!qMsNvr->isSlaveMode()) {
        ui->comboBox_playType->addItem(GET_TEXT("PLAYBACK/80066", "Event Playback"), EventPlayback);
        ui->comboBox_playType->addItem(GET_TEXT("PLAYBACK/80067", "Tag Playback"), TagPlayback);
        ui->comboBox_playType->addItem(GET_TEXT("PLAYBACK/80123", "Split Playback"), SplitPlayback);
        ui->comboBox_playType->addItem(GET_TEXT("PLAYBACK/80031", "Picture Playback"), PicturePlayback);
    }

    ui->comboBox_stream->clear();
    ui->comboBox_stream->addItem(GET_TEXT("IMAGE/37333", "Primary Stream"), FILE_TYPE_MAIN);
    ui->comboBox_stream->addItem(GET_TEXT("CHANNELMANAGE/30057", "Secondary Stream"), FILE_TYPE_SUB);

    ui->toolButton_close->setText(GET_TEXT("CHANNELMANAGE/30006", "Live View"));
}

void PlaybackMode::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    const MsPlaybackType &mode = playbackType();
    switch (mode) {
    case GeneralPlayback:
        ui->page_general->dealMessage(message);
        break;
    case EventPlayback:
        ui->page_event->dealMessage(message);
        break;
    case TagPlayback:
        ui->page_tag->dealMessage(message);
        break;
    case SplitPlayback:
        ui->page_split->dealMessage(message);
        break;
    case PicturePlayback:
        ui->page_picture->dealMessage(message);
        break;
    }
}

void PlaybackMode::on_comboBox_playType_activated(int index)
{
    if (isFisheyeDewarpEnable()) {
        PlaybackWindow::instance()->closeFisheyeDewarp();
    }
    //
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_CLEAR_TEMP_TAGS_PLAYBACK, NULL, 0);
    //
    const MsPlaybackType &mode = MsPlaybackType(ui->comboBox_playType->itemData(index).toInt());
    //
    s_smartSpeed->closeSmartSpeed();
    if (playbackType() == GeneralPlayback) {
        ui->page_general->closePlayback();
    }
    if (playbackType() == SplitPlayback) {
        ui->page_split->closePlayback();
    }
    if (mode == PicturePlayback) {
        ui->comboBox_stream->setVisible(false);
    } else {
        ui->comboBox_stream->setVisible(true);
    }
    //channel show -
    setCurrentTimeLine(-1);
    //
    setPlaybackType(mode);
    emit playbackModeChanged(mode);
    //
    switch (mode) {
    case GeneralPlayback:
        ui->stackedWidget->setCurrentWidget(ui->page_general);
        ui->page_general->initializeData();
        break;
    case EventPlayback:
        ui->stackedWidget->setCurrentWidget(ui->page_event);
        ui->page_event->initializeData();
        break;
    case TagPlayback:
        ui->stackedWidget->setCurrentWidget(ui->page_tag);
        ui->page_tag->initializeData();
        break;
    case SplitPlayback:
        ui->stackedWidget->setCurrentWidget(ui->page_split);
        ui->page_split->initializeData();
        break;
    case PicturePlayback:
        ui->stackedWidget->setCurrentWidget(ui->page_picture);
        ui->page_picture->initializeData();
        break;
    }
}

void PlaybackMode::on_comboBox_stream_activated(int index)
{
    if (isFisheyeDewarpEnable()) {
        PlaybackWindow::instance()->closeFisheyeDewarp();
    }

    //
    const FILE_TYPE_EN &stream = FILE_TYPE_EN(ui->comboBox_stream->itemData(index).toInt());
    setPlaybackStream(stream);

    //如果当前有选中的通道，先停止，然后重新查询、播放
    gPlaybackEventData.clearAll();
    gPlaybackTagData.clearAll();
    const QList<int> &channelList = channelCheckedList();
    if (!channelList.isEmpty()) {
        //showWait();
        getMonthEvent();
        waitForStopAllPlayback();
        closeAllCommonPlayback();
        closeAllEventPlayback();
        bool hasRecord = false;
        for (int i = 0; i < channelList.size(); ++i) {
            const int &channel = channelList.at(i);
            waitForSearchCommonPlayback(channel);
            bool hasCommonBackup = isChannelHasCommonBackup(channel);
            hasRecord |= hasCommonBackup;
        }
        if (hasRecord) {
            waitForStartAllPlayback();
            if (playbackType() == GeneralPlayback && !isSmartSearchMode()) {
                waitForSearchGeneralEventPlayBack();
                if (getFilterEvent() != INFO_MAJOR_NONE && !gPlaybackEventData.hasEventBackup(currentChannel(), static_cast<uint>(getFilterEvent()))) {
                    ShowMessageBox(GET_TEXT("PLAYBACK/167001", "No data."));
                }
                if (getIsEventOnly()) {
                    s_smartSpeed->startEventPlayBack();
                }
            }
        } else {
            if (playbackType() == GeneralPlayback) {
                if (playbackStream() == FILE_TYPE_MAIN) {
                    ShowMessageBox(GET_TEXT("PLAYBACK/167002", "There are no Primary recording files at this time on this channel."));
                } else if (playbackStream() == FILE_TYPE_SUB) {
                    ShowMessageBox(GET_TEXT("PLAYBACK/167003", "There are no Secondary recording files at this time on this channel."));
                }
            }
            s_playbackBar->setPlaybackButtonState(PlaybackState_None);
        }
        //closeWait();
    }
    //
    s_playbackBar->updateTimeLine();
}

void PlaybackMode::onToolButtonCloseClicked()
{
    PlaybackWindow::instance()->closePlayback();
}
