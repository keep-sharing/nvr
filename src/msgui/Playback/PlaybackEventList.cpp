#include "PlaybackEventList.h"
#include "ui_PlaybackList.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "PlaybackBar.h"
#include "PlaybackEvent.h"
#include "PlaybackWindow.h"
#include "centralmessage.h"
#include <QtDebug>
#include <qmath.h>

PlaybackEventList::PlaybackEventList(QWidget *parent)
    : PlaybackList(parent)
{
    ui->widget_button->hide();

    //
    initializeTreeView();
}

void PlaybackEventList::closePlayback()
{
    on_toolButton_close_clicked();
}

NetworkResult PlaybackEventList::dealNetworkCommond(const QString &commond)
{
    qDebug() << "PlaybackEventList::dealNetworkCommond," << commond;

    NetworkResult result = NetworkReject;
    if (commond.startsWith("Video_Play")) {
        playItem(m_currentRow);
        result = NetworkAccept;
    } else if (commond.startsWith("Dial_Insid_Add")) {
        //选中下一个
        selectNext();
        result = NetworkAccept;
    } else if (commond.startsWith("Dial_Insid_Sub")) {
        //选中上一个
        selectPrevious();
        result = NetworkAccept;
    }

    return result;
}

void PlaybackEventList::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    if (!isVisible()) {
        return;
    }
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN:
        ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(message);
        break;
    case RESPONSE_FLAG_START_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_START_ALL_PLAYBACK(message);
        break;
    case RESPONSE_FLAG_STOP_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(message);
        break;
    default:
        break;
    }
}

void PlaybackEventList::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_EVT_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(message);
        break;
    case RESPONSE_FLAG_GET_SEARCH_EVT_PAGE:
        ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(message);
        break;
    default:
        break;
    }
}

void PlaybackEventList::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(MessageReceive *message)
{
    m_pageIndex = 0;
    m_pageCount = 0;
    m_searchSid = -1;

    //
    struct resp_search_event_backup *event_backup_array = (struct resp_search_event_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_event_backup);
    if (!event_backup_array) {
        qWarning() << "PlaybackEventList::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP, data is null.";
        return;
    }

    m_backupList.clear();
    for (int i = 0; i < count; ++i) {
        m_backupList.append(event_backup_array[i]);
    }

    const resp_search_event_backup &firstBackup = event_backup_array[0];
    m_searchSid = firstBackup.sid;
    //page
    m_allCount = firstBackup.allCnt;
    if (m_allCount > MAX_SEARCH_BACKUP_COUNT) {
        m_allCount = MAX_SEARCH_BACKUP_COUNT;
    }
    m_pageCount = qCeil(m_allCount / 100.0);
    int backupArrayPage = qCeil(count / 100.0);
    if (m_pageCount > backupArrayPage) {
        for (int i = backupArrayPage; i < m_pageCount; ++i) {
            struct rep_get_search_backup pageinfo;
            memset(&pageinfo, 0, sizeof(struct rep_get_search_backup));
            pageinfo.sid = firstBackup.sid;
            pageinfo.npage = i + 1;
            qDebug() << QString("REQUEST_FLAG_GET_SEARCH_EVT_PAGE, page: %1").arg(pageinfo.npage);
            sendMessage(REQUEST_FLAG_GET_SEARCH_EVT_PAGE, &pageinfo, sizeof(struct rep_get_search_backup));
            //m_eventLoop.exec();
        }
    }
    ui->lineEdit_page->setMaxPage(m_pageCount);
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_allCount).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    updateTreeList();
}

void PlaybackEventList::ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(MessageReceive *message)
{
    //
    struct resp_search_event_backup *event_backup_array = (struct resp_search_event_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_event_backup);
    if (!event_backup_array) {
        qWarning() << "PlaybackEventList::ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE, data is null.";
        return;
    }
    for (int i = 0; i < count; ++i) {
        m_backupList.append(event_backup_array[i]);
    }
    m_eventLoop.exit();
}

QStringList PlaybackEventList::treeHeaders()
{
    QStringList headerList;
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("EVENTBACKUP/101009", "Event Duration Time");
    headerList << GET_TEXT("PLAYBACK/80089", "Play");
    return headerList;
}

void PlaybackEventList::updateTreeList()
{
    ui->treeView->clearContent();
    ui->treeView->setHorizontalHeaderLabels(treeHeaders());
    int beginIndex = m_pageIndex * 100;
    int endIndex = beginIndex + 100;
    if (endIndex > m_backupList.count()) {
        endIndex = m_backupList.count();
    }
    ui->treeView->setRowCount(endIndex - beginIndex);
    for (int i = beginIndex, row = 0; i < endIndex; ++i) {
        const resp_search_event_backup &event_backup = m_backupList.at(i);
        ui->treeView->setItemText(row, ColumnChannel, QString::number(event_backup.chnid + 1));
        //const QDateTime &startDateTime = QDateTime::fromString(event_backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        const QDateTime &endDateTime = QDateTime::fromString(event_backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
        ui->treeView->setItemText(row, ColumnTime, QString("%1-%2").arg(event_backup.pStartTime).arg(endDateTime.toString("HH:mm:ss")));

        ItemKey itemKey(row, m_pageIndex);
        if (itemKey == m_currentItemKey) {
            setItemTextColor(row, "#0AA8E3");
        }
        row++;
    }
}

void PlaybackEventList::playItem(int row)
{
    if (row < 0) {
        return;
    }
    m_currentRow = row;
    //showWait();
    //
    if (m_currentPlaySid >= 0) {
        waitForStopAllPlayback();
        closeAllCommonPlayback();
        m_currentPlaySid = -1;
    }
    //
    int index = m_pageIndex * 100 + row;
    const resp_search_event_backup &backup = m_backupList.at(index);
    int channel = backup.chnid;
    clearChannelChecked();
    setChannelChecked(channel, true);
    updateLayout(channel);
    //
    const QDateTime &startDateTime = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
    const QDateTime &endDateTime = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
    setPlaybackDate(startDateTime.date());
    waitForSearchCommonPlayback(channel, startDateTime, endDateTime);
    waitForStartAllPlayback();
    //
    setCurrentTimeLine();
    setTimeLineStartDateTime(startDateTime);
    //closeWait();

    m_currentItemKey = ItemKey(row, m_pageIndex);
    clearAllItemTextColor();
    setItemTextColor(row, "#0AA8E3");
}

void PlaybackEventList::searchEventBackupPage(int page)
{
    Q_UNUSED(page)
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_allCount).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    updateTreeList();
}

void PlaybackEventList::onItemClicked(const QModelIndex &index)
{
    m_currentRow = index.row();

    switch (index.column()) {
    case ColumnPlay: {
        playItem(index.row());
        break;
    }
    default:
        break;
    }
}

void PlaybackEventList::onItemDoubleClicked(const QModelIndex &index)
{
    playItem(index.row());
}

void PlaybackEventList::on_toolButton_firstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    searchEventBackupPage(m_pageIndex + 1);
}

void PlaybackEventList::on_toolButton_previousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    searchEventBackupPage(m_pageIndex + 1);
}

void PlaybackEventList::on_toolButton_nextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    searchEventBackupPage(m_pageIndex + 1);
}

void PlaybackEventList::on_toolButton_lastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    searchEventBackupPage(m_pageIndex + 1);
}

void PlaybackEventList::on_pushButton_go_clicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;
    searchEventBackupPage(m_pageIndex + 1);
}

void PlaybackEventList::on_toolButton_close_clicked()
{
    if (isFisheyeDewarpEnable()) {
        PlaybackWindow::instance()->closeFisheyeDewarp();
    }
    //
    //showWait();
    if (m_currentPlaySid >= 0) {
        waitForStopAllPlayback();
        closeAllCommonPlayback();
        m_currentPlaySid = -1;
    }
    //
    m_backupList.clear();
    if (m_searchSid >= 0) {
        m_searchSid = -1;
    }
    //
    clearChannelChecked();
    m_currentRow = -1;
    //channel show -
    setCurrentTimeLine(-1);
    emit updateLayout(-1);
    //closeWait();
    close();

    s_playbackBar->setPlaybackButtonState(PlaybackState_None);
}
