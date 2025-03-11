#include "PlaybackGeneral.h"
#include "ui_PlaybackGeneral.h"
#include "EventLoop.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PlaybackBar.h"
#include "PlaybackEventData.h"
#include "PlaybackLayout.h"
#include "PlaybackTagData.h"
#include "PlaybackTimeLine.h"
#include "PlaybackWindow.h"
#include "SmartSearchControl.h"
#include "SmartSpeedPanel.h"
#include "centralmessage.h"
#include "msuser.h"
#include <QElapsedTimer>
#include <QKeyEvent>

extern "C" {

}

const int MaxPlayback = 16;

PlaybackGeneral::PlaybackGeneral(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::PlaybackGeneral)
{
    ui->setupUi(this);
    BasePlayback::s_playbackGeneral = this;

    //channel tree
    QStringList headerList;
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("COMMON/1051", "Name");
    ui->treeView_channel->setHorizontalHeaderLabels(headerList);
    ui->treeView_channel->setColumnWidth(0, 50);
    ItemChannelDelegate *itemDelegate = new ItemChannelDelegate(this);
    itemDelegate->setCheckedPixmap(":/common/common/checkbox-checked.png");
    itemDelegate->setUncheckedPixmap(":/common/common/checkbox-unchecked.png");
    ui->treeView_channel->setItemDelegateForColumn(0, itemDelegate);
    //
    connect(ui->treeView_channel, SIGNAL(itemClicked(int, int)), this, SLOT(onItemClicked(int, int)));
    connect(ui->treeView_channel, SIGNAL(enterPressed()), this, SLOT(onTreeViewEnterPressed()));

    //日历
    connect(ui->calendarWidget, SIGNAL(dateChanged(QDate)), this, SLOT(onSelectedDateChanged(QDate)));

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PlaybackGeneral::~PlaybackGeneral()
{
    delete ui;
}

void PlaybackGeneral::initializeData()
{
    const QDate &currentDate = QDate::currentDate();
    ui->calendarWidget->setCurrentDate(currentDate);
    setPlaybackDate(currentDate);
    s_playbackTimeLine->setTimeLineBeginDateTime(QDateTime(currentDate, QTime(0, 0)));

    //
    struct osd osd_array[MAX_CAMERA];
    int osdCount = 0;
    read_osds(SQLITE_FILE_NAME, osd_array, &osdCount);
    //
    QList<int> channelList = gMsUser.accessiblePlaybackChannelList();
    ui->treeView_channel->setRowCount(channelList.size());
    int row = 0;
    for (int i = 0; i < channelList.size(); ++i) {
        int channel = channelList.at(i);
        struct osd &osd_info = osd_array[channel];
        ui->treeView_channel->setItemData(row, 0, false, ItemCheckedRole);
        ui->treeView_channel->setItemData(row, 0, QString::number(channel + 1), ItemTextRole);
        ui->treeView_channel->setItemChannel(row, channel);
        ui->treeView_channel->setItemText(row, 1, osd_info.name);
        ui->treeView_channel->setItemToolTip(row, 1, osd_info.name);
        row++;
    }
    ui->treeView_channel->setCurrentRow(0);
    onItemClicked(0, 1);
}

void PlaybackGeneral::closePlayback()
{
    on_pushButton_clearAll_clicked();
}

void PlaybackGeneral::setCurrentSelectedChannel(int channel)
{
    ui->treeView_channel->setCurrentRow(channel);
}

bool PlaybackGeneral::hasFocus()
{
    bool result = false;
    if (ui->treeView_channel->hasFocus()) {
        result = true;
    } else if (ui->pushButton_selectMax->hasFocus()) {
        result = true;
    } else if (ui->pushButton_clearAll->hasFocus()) {
        result = true;
    }
    return result;
}

bool PlaybackGeneral::focusPrevious()
{
    bool result = true;
    if (ui->treeView_channel->hasFocus()) {
        result = false;
    } else if (ui->pushButton_selectMax->hasFocus()) {
        ui->treeView_channel->setFocus();
    } else if (ui->pushButton_clearAll->hasFocus()) {
        ui->pushButton_selectMax->setFocus();
    } else {
        ui->pushButton_clearAll->setFocus();
    }
    return result;
}

bool PlaybackGeneral::focusNext()
{
    bool result = true;
    if (ui->pushButton_clearAll->hasFocus()) {
        result = false;
    } else if (ui->pushButton_selectMax->hasFocus()) {
        ui->pushButton_clearAll->setFocus();
    } else if (ui->treeView_channel->hasFocus()) {
        ui->pushButton_selectMax->setFocus();
    } else {
        ui->treeView_channel->setFocus();
    }
    return result;
}

NetworkResult PlaybackGeneral::dealNetworkCommond(const QString &commond)
{
    qDebug() << "====PlaybackGeneral::dealNetworkCommond====";
    qDebug() << "----commond:" << commond;
    NetworkResult result = NetworkReject;
    if (commond.startsWith("ChangeFocus_")) {
        if (ui->treeView_channel->hasFocus()) {
            int row = ui->treeView_channel->currentIndex().row();
            if (row == ui->treeView_channel->rowCount() - 1) {
                ui->pushButton_selectMax->setFocus();
            } else {
                selectPreviousChannel();
            }
        }
        result = NetworkAccept;
    } else if (commond.startsWith("ChangeFocus_Prev")) {
        if (ui->treeView_channel->hasFocus()) {
            int row = ui->treeView_channel->currentIndex().row();
            if (row == 0) {
                focusPreviousChild();
            } else {
                selectNextChannel();
            }
        }
        result = NetworkAccept;
    } else if (commond.startsWith("Enter")) {
        if (ui->treeView_channel->hasFocus()) {
            int row = ui->treeView_channel->currentIndex().row();
            if (row < 0) {
                result = NetworkReject;
            } else {
                onItemClicked(row, 0);
                result = NetworkAccept;
            }
        }
    }
    return result;
}

void PlaybackGeneral::dealMessage(MessageReceive *message)
{
    BasePlayback::dealMessage(message);

    switch (message->type()) {
    case RESPONSE_FLAG_GET_MONTH_EVENT:
        ON_RESPONSE_FLAG_GET_MONTH_EVENT(message);
        break;
    }
}

void PlaybackGeneral::ON_RESPONSE_FLAG_GET_MONTH_EVENT(MessageReceive *message)
{
    qDebug() << "====PlaybackGeneral::ON_RESPONSE_FLAG_GET_MONTH_EVENT====";
    struct pb_month_t *month = (struct pb_month_t *)message->data;
    if (!month) {
        qWarning() << "----data: null";
        return;
    }

    QDate currentDate = ui->calendarWidget->selectedDate();
    int dayCount = currentDate.daysInMonth();
    QMap<QDate, QColor> dayColorMap;
    for (int i = 0; i < dayCount; ++i) {
        //qDebug() << "----" << i << ":" << month->enEvent[i];
        if (month->enEvent[i] == 0) {
            continue;
        }
        QDate date(currentDate.year(), currentDate.month(), i + 1);
        if (month->enEvent[i] == REC_EVENT_TIME) {
            dayColorMap.insert(date, QColor("#09A8E0"));
        } else {
            dayColorMap.insert(date, QColor("#E70000"));
        }
    }
    ui->calendarWidget->setDaysColor(dayColorMap);
}

void PlaybackGeneral::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "====PlaybackGeneral::keyPressEvent====";
    qDebug() << "----this:" << this;
    qDebug() << "----event:" << event;
    switch (event->key()) {
    case Qt::Key_Escape:
        //escapePressed();
        event->accept();
        break;
    case Qt::Key_Return:
        returnPressed();
        event->accept();
        break;
    default:
        break;
    }
    BasePlayback::keyPressEvent(event);
}

void PlaybackGeneral::selectNextChannel()
{
    int channel = ui->treeView_channel->currentIndex().row();
    if (channel < 0) {
        channel = 0;
    } else if (channel < ui->treeView_channel->rowCount() - 1) {
        channel++;
    }
    ui->treeView_channel->setCurrentRow(channel);
    setCurrentChannel(channel);
    setCurrentVideoInLayout(channel);
    setCurrentTimeLine(channel);
}

void PlaybackGeneral::selectPreviousChannel()
{
    int channel = ui->treeView_channel->currentIndex().row();
    if (channel == 0) {
        return;
    }
    if (channel < 0) {
        channel = 0;
    } else if (channel > 0) {
        channel--;
    }
    ui->treeView_channel->setCurrentRow(channel);
    setCurrentChannel(channel);
    setCurrentVideoInLayout(channel);
    setCurrentTimeLine(channel);
}

void PlaybackGeneral::selectChannel(int channel)
{
    ui->treeView_channel->setCurrentRow(channel);
    setCurrentChannel(channel);
    setCurrentVideoInLayout(channel);
    setCurrentTimeLine(channel);
}

void PlaybackGeneral::returnPressed()
{
    if (ui->treeView_channel->hasFocus()) {
        int row = ui->treeView_channel->currentIndex().row();
        if (row < 0) {

        } else {
            onItemClicked(row, 0);
        }
    }
}

void PlaybackGeneral::onLanguageChanged()
{
    QStringList headerList;
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("COMMON/1051", "Name");
    ui->treeView_channel->setHorizontalHeaderLabels(headerList);
    //
    ui->label_general->setText(GET_TEXT("PLAYBACK/80130", "General"));
    ui->label_event->setText(GET_TEXT("PLAYBACK/80131", "Event"));
    //
    ui->pushButton_selectMax->setText(GET_TEXT("PLAYBACK/80112", "Select Max"));
    ui->pushButton_clearAll->setText(GET_TEXT("COMMON/1021", "Clear All"));
}

void PlaybackGeneral::onSearchFinished(int channel)
{
    SearchCommonBackup *search = m_searchMap.value(channel);
    if (!search) {
        qMsCritical() << "search is nullptr, channel:" << channel;
        return;
    }
    PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
    auto list = search->backupList();
    for (int i = 0; i < list.size(); ++i) {
        const auto &backup = list.at(i);
        channelInfo.appendCommonBackup(backup);
    }

    m_searchingMap.remove(channel);
    if (m_searchingMap.isEmpty()) {
        gEventLoopExit(0);
    }
}

void PlaybackGeneral::onStartFinished(int channel)
{
    s_playbackBar->onChannelStarted(channel);

    m_startingMap.remove(channel);
    if (m_startingMap.isEmpty()) {
        gEventLoopExit(0);
    }
}

void PlaybackGeneral::onStopFinished(int channel)
{
    s_playbackBar->onChannelStoped(channel);

    m_stoppingMap.remove(channel);
    if (m_stoppingMap.isEmpty()) {
        gEventLoopExit(0);
    }
}

void PlaybackGeneral::onCloseFinished(int channel)
{
    m_closingMap.remove(channel);
    if (m_closingMap.isEmpty()) {
        gEventLoopExit(0);
    }
}

void PlaybackGeneral::onItemClicked(int row, int column)
{
    const int &channel = ui->treeView_channel->itemChannel(row);
    //
    switch (column) {
    case 0: {
        if (!gMsUser.checkPlaybackChannelPermission(channel)) {
            ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
            return;
        }
        //
        bool checked = ui->treeView_channel->isItemChecked(row);
        checked = !checked;
        if (checked && channelCheckedCount() >= 16) {
            return;
        }
        //
        if (checked && channelCheckedCount() >= 1) {
            if (s_smartSpeed->isSmartPlaySpeedEnable() || getFilterEvent() != INFO_MAJOR_NONE || SmartSearchControl::instance()->isSmartSearchMode()) {
                ShowMessageBox(GET_TEXT("FISHEYE/12006", "Only available when playback with single channel."));
                return;
            }
        }
        PlaybackWindow::instance()->closeFisheyeDewarp();
        PlaybackZoom::instance()->closeZoom();
        PlaybackZoom::instance()->setZoomMode(false);
        //
        ui->treeView_channel->setItemChecked(row, checked);
        //
        setChannelChecked(channel, checked);
        updateLayout(channel);
        //
        getMonthEvent();
        //showWait();
        if (checked) {
            //
            s_smartSpeed->clearTimeInfo();
            //
            if (playbackState() == PlaybackState_Pause) {
                //有暂停的，先恢复播放
                waitForResumeAllPlayback();
            }
            waitForSearchCommonPlayback(channel);
            if (isChannelHasCommonBackup(channel)) {
                waitForStartPlayback(channel);
                if (channelCheckedCount() > 1) {
                    seekPlayback();
                }
                if (!isSmartSearchMode()) {
                    waitForSearchGeneralEventPlayBack();
                    if (getFilterEvent() != INFO_MAJOR_NONE && !gPlaybackEventData.hasEventBackup(currentChannel(), static_cast<uint>(getFilterEvent()))) {
                        ShowMessageBox(GET_TEXT("PLAYBACK/167001", "No data."));
                    }
                    if (getIsEventOnly()) {
                        s_smartSpeed->startEventPlayBack();
                    }
                }
            } else {
                if (playbackStream() == FILE_TYPE_MAIN) {
                    ShowMessageBox(GET_TEXT("PLAYBACK/167002", "There are no Primary recording files at this time on this channel."));
                } else if (playbackStream() == FILE_TYPE_SUB) {
                    ShowMessageBox(GET_TEXT("PLAYBACK/167003", "There are no Secondary recording files at this time on this channel."));
                }
            }
            setCurrentTimeLine(channel);
        } else {
            //
            waitForStopPlayback(channel);
            closeCommonPlayback(channel);
            closeEventPlayback(channel);
            gPlaybackEventData.clear(channel);
            gPlaybackTagData.clear(channel);
            //选中第一个
            const QList<int> &channelList = channelCheckedList();
            if (!channelList.isEmpty()) {
                int channel = channelList.first();
                ui->treeView_channel->setCurrentRow(channel);
                setCurrentChannel(channel);
                setCurrentVideoInLayout(channel);
                setCurrentTimeLine(channel);
            } else {
                setCurrentTimeLine(-1);
            }
        }
        //closeWait();
        break;
    }
    case 1: {
        const QList<int> &channelList = channelCheckedList();
        if (channelList.indexOf(channel) != -1) {
            setCurrentChannel(channel);
            setCurrentVideoInLayout(channel);
            setCurrentTimeLine(channel);
        }
        break;
    }
    default:
        break;
    }
}

void PlaybackGeneral::onTreeViewEnterPressed()
{
    int row = ui->treeView_channel->currentIndex().row();
    if (row < 0) {

    } else {
        onItemClicked(row, 0);
    }
}

void PlaybackGeneral::onSelectedDateChanged(const QDate &date)
{
    //
    if (PlaybackZoom::instance()->isZooming()) {
        PlaybackZoom::instance()->closeZoom();
    }
    if (PlaybackZoom::instance()->isZoomMode()) {
        PlaybackZoom::instance()->setZoomMode(false);
    }

    //
    setPlaybackDate(date);
    setPlaybackTime(QTime(0, 0));
    s_playbackTimeLine->setTimeLineBeginDateTime(QDateTime(date, QTime(0, 0)));
    s_playbackBar->setCurrentDateTime(QDateTime(date, QTime(0, 0)));
    ui->calendarWidget->clearDaysColor();

    //如果当前有选中的通道，先停止，然后重新查询、播放
    const QList<int> &channelList = channelCheckedList();
    if (!channelList.isEmpty()) {
        //重新查询日历
        getMonthEvent();

        //showWait();
        waitForStopAllPlayback();
        closeAllCommonPlayback();
        closeAllEventPlayback();
        gPlaybackEventData.clearAll();
        bool hasRecord = false;
        for (int i = 0; i < channelList.size(); ++i) {
            const int &channel = channelList.at(i);
            waitForSearchCommonPlayback(channel);
            bool hasCommonBackup = isChannelHasCommonBackup(channel);
            hasRecord |= hasCommonBackup;
        }
        if (hasRecord) {
            waitForStartAllPlayback();
            if (!isSmartSearchMode()) {
                waitForSearchGeneralEventPlayBack();
                if (getFilterEvent() != INFO_MAJOR_NONE && !gPlaybackEventData.hasEventBackup(currentChannel(), static_cast<uint>(getFilterEvent()))) {
                    ShowMessageBox(GET_TEXT("PLAYBACK/167001", "No data."));
                }
                if (getIsEventOnly()) {
                    s_smartSpeed->startEventPlayBack();
                }
            }
        } else {
            if (channelList.size() == 1) {
                if (playbackStream() == FILE_TYPE_MAIN) {
                    ShowMessageBox(GET_TEXT("PLAYBACK/167002", "There are no Primary recording files at this time on this channel."));
                } else if (playbackStream() == FILE_TYPE_SUB) {
                    ShowMessageBox(GET_TEXT("PLAYBACK/167003", "There are no Secondary recording files at this time on this channel."));
                }
            }
            s_playbackBar->setPlaybackButtonState(PlaybackState_None);
        }
        //
        if (hasRecord && s_smartSearch->isSmartSearchMode()) {
            s_smartSearch->research();
        } else {
            s_smartSearch->closeSmartSearch();
            //closeWait();
        }
    }

    //
    PlaybackLayout::instance()->clearNoResource();
}

void PlaybackGeneral::on_pushButton_selectMax_clicked()
{
    ui->pushButton_selectMax->clearUnderMouse();

    qMsDebug() << "begin";
    if ((s_smartSpeed->isSmartPlaySpeedEnable() || s_smartSearch->isSmartSearchMode()) || getFilterEvent() != INFO_MAJOR_NONE) {
        ShowMessageBox(GET_TEXT("FISHEYE/12006", "Only available when playback with single channel."));
        qMsDebug() << "return 1";
        return;
    }
    //
    QList<int> channelList;
    for (int row = 0; row < ui->treeView_channel->rowCount(); ++row) {
        if (!ui->treeView_channel->isItemChecked(row)) {
            if (channelCheckedList().size() >= 16) {
                break;
            }
            const int &channel = ui->treeView_channel->itemChannel(row);
            if (!gMsUser.checkPlaybackChannelPermission(channel)) {
                continue;
            }
            ui->treeView_channel->setItemChecked(row, true);
            setChannelChecked(channel, true);

            channelList.append(channel);
        }
    }
    if (channelList.isEmpty()) {
        qMsDebug() << "return 2";
        return;
    }

    //设置布局
    const int channel = channelList.first();
    ui->treeView_channel->setCurrentRow(channel);
    updateLayout(channel);
    setCurrentChannel(channel);
    //查询日历
    getMonthEvent();
    //有暂停的，先恢复播放
    if (playbackState() == PlaybackState_Pause) {
        waitForResumeAllPlayback();
    }
    //
    //showWait();

    QElapsedTimer timer;
    timer.start();
#if 1
    for (int i = 0; i < channelList.size(); ++i) {
        const int &channel = channelList.at(i);
        m_searchingMap.insert(channel, 0);

        req_search_common_backup common_backup;
        memset(&common_backup, 0, sizeof(req_search_common_backup));
        makeChannelMask(channel, common_backup.chnMaskl, sizeof(common_backup.chnMaskl));
        common_backup.chnNum = 1;
        common_backup.enType = playbackStream();
        common_backup.enEvent = REC_EVENT_ALL;
        common_backup.enState = SEG_STATE_ALL;
        snprintf(common_backup.pStartTime, sizeof(common_backup.pStartTime), "%s 00:00:00", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
        snprintf(common_backup.pEndTime, sizeof(common_backup.pEndTime), "%s 23:59:59", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
        common_backup.all = MF_YES;
        //

        SearchCommonBackup *search = m_searchMap.value(channel);
        if (!search) {
            search = new SearchCommonBackup(this);
            connect(search, SIGNAL(finished(int)), this, SLOT(onSearchFinished(int)));
        }
        search->setSearchInfo(common_backup);
        search->startSearch();
        m_searchMap.insert(channel, search);
    }
    gEventLoop.exec();
    qMsDebug() << "thread search took" << timer.restart() << "ms";
#if 0
    set_local_pb_sync(false);
    for (int i = 0; i < channelList.size(); ++i) {
        const int &channel = channelList.at(i);

        rep_start_all_play start_all;
        memset(&start_all, 0, sizeof(rep_start_all_play));
        start_all.actState          = PB_PLAY;
        start_all.actSpeed          = playbackSpeed();
        start_all.actdir            = playbackDirection();
        makeChannelMask(channel, start_all.chnMaskl, sizeof(start_all.chnMaskl));
        snprintf(start_all.pPlayTime, sizeof(start_all.pPlayTime), "%s", QDateTime(playbackDate(), playbackTime()).toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
        snprintf(start_all.pStartTime, sizeof(start_all.pStartTime), "%s 00:00:00", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
        snprintf(start_all.pEndTime, sizeof(start_all.pEndTime), "%s 23:59:59", playbackDate().toString("yyyy-MM-dd").toStdString().c_str());

        auto *start = m_startMap.value(channel);
        if (!start) {
            start = new StartPlayback(this);
            connect(start, SIGNAL(finished(int)), this, SLOT(onStartFinished(int)));
            m_startMap.insert(channel, start);
        }
        start->start(start_all);
        m_startingMap.insert(channel, 0);
    }
    gEventLoop.exec();
    set_local_pb_sync(true);
#endif
    bool hasRecord = false;
    QString noRecordChannels;
    for (int i = 0; i < channelList.size(); ++i) {
        const int &channel = channelList.at(i);
        bool hasCommonBackup = isChannelHasCommonBackup(channel);
        hasRecord |= hasCommonBackup;
        if (!hasCommonBackup) {
            noRecordChannels += ui->treeView_channel->itemText(channel, 1);
            noRecordChannels += ", ";
        }
    }
    if (!noRecordChannels.isEmpty()) {
        noRecordChannels.remove(noRecordChannels.size() - 2, 2);
        noRecordChannels += ".";
        if (playbackStream() == FILE_TYPE_MAIN) {
            ShowMessageBox(QString(GET_TEXT("PLAYBACK/167004", "There are no Primary recording files at this time on the %1")).arg(noRecordChannels));
        } else if (playbackStream() == FILE_TYPE_SUB) {
            ShowMessageBox(QString(GET_TEXT("PLAYBACK/167005", "There are no Secondary recording files at this time on the %1")).arg(noRecordChannels));
        }
    }

    if (hasRecord) {
        waitForStartAllPlayback();
    }
    qMsDebug() << "start play took" << timer.restart() << "ms";
#else
    bool hasRecord = false;
    for (int i = 0; i < channelList.size(); ++i) {
        const int &channel = channelList.at(i);
        waitForSearchCommonPlayback(channel);
        bool hasCommonBackup = isChannelHasCommonBackup(channel);
        hasRecord |= hasCommonBackup;
    }
    qMsDebug() << "normal search took" << timer.restart() << "ms";
    if (hasRecord) {
        waitForStartAllPlayback();
    }
    qMsDebug() << "start play took" << timer.restart() << "ms";
#endif

    //closeWait();
    qMsDebug() << "end";
}

void PlaybackGeneral::on_pushButton_clearAll_clicked()
{
    ui->pushButton_clearAll->clearUnderMouse();
    setFilterEvent(0);
    setIsEventOnly(false);

    qMsDebug() << "begin";
    if (isFisheyeDewarpEnable()) {
        PlaybackWindow::instance()->closeFisheyeDewarp();
    }
    //
    QList<int> channelList = channelCheckedList();
    if (channelList.isEmpty()) {
        qMsDebug() << "return 1";
        return;
    }

    ui->calendarWidget->clearDaysColor();

    //
    PlaybackZoom::instance()->closeZoom();

    //等待停止完成
    //showWait();

    QElapsedTimer timer;
    timer.start();

#if 0
    for (int i = 0; i < channelList.size(); ++i) {
        const int &channel = channelList.at(i);
        StopPlayback *stop = m_stopMap.value(channel);
        if (!stop) {
            stop = new StopPlayback(this);
            connect(stop, SIGNAL(finished(int)), this, SLOT(onStopFinished(int)));
            m_stopMap.insert(channel, stop);
        }
        stop->stop(channel);
        m_stoppingMap.insert(channel, 0);
    }
    gEventLoop.exec();
    qMsDebug() << "stop play took" << timer.restart() << "ms";
    //
    for (auto iter = s_channelInfoMap.begin(); iter != s_channelInfoMap.end(); ++iter) {
        PlaybackChannelInfo &channelInfo = iter.value();
        const int &channel = channelInfo.channel();
        const int &sid = channelInfo.sid();
        if (sid >= 0) {
            channelInfo.clearCommonBackup();

            CloseCommonBackup *close = m_closeMap.value(channel);
            if (!close) {
                close = new CloseCommonBackup(this);
                connect(close, SIGNAL(finished(int)), this, SLOT(onCloseFinished(int)));
                m_closeMap.insert(channel, close);
            }
            close->close(channel, sid);
            m_closingMap.insert(channel, sid);
        }
    }
    gEventLoop.exec();
    qMsDebug() << "close play took" << timer.restart() << "ms";
#else
    qMsDebug() << "waitForStopAllPlayback";
    waitForStopAllPlayback();
    qMsDebug() << "closeAllCommonPlayback";
    closeAllCommonPlayback();
    qMsDebug() << "closeAllEventPlayback";
    closeAllEventPlayback();
#endif
    //
    for (int row = 0; row < ui->treeView_channel->rowCount(); ++row) {
        if (ui->treeView_channel->isItemChecked(row)) {
            ui->treeView_channel->setItemChecked(row, false);
            const int &channel = ui->treeView_channel->itemChannel(row);
            setChannelChecked(channel, false);
        }
    }
    //
    gPlaybackEventData.clearAll();
    gPlaybackTagData.clearAll();
    initializeTimeLine();
    setCurrentTimeLine(-1);
    //更新布局
    updateLayout(-1);
    //closeWait();
    qMsDebug() << "end";
}
