#include "PlaybackTagList.h"
#include "ui_PlaybackList.h"
#include "centralmessage.h"
#include "MsLanguage.h"
#include "PlaybackWindow.h"
#include "PlaybackBar.h"
#include "PlaybackTag.h"
#include <QtDebug>
#include <qmath.h>
#include "MessageBox.h"
#include "MsDevice.h"

PlaybackTagList::PlaybackTagList(QWidget *parent)
    : PlaybackList(parent)
{
    ui->widget_button->hide();
    //
    initializeTreeView();
}

void PlaybackTagList::closePlayback()
{
    on_toolButton_close_clicked();
}

NetworkResult PlaybackTagList::dealNetworkCommond(const QString &commond)
{
    qDebug() << "PlaybackTagList::dealNetworkCommond," << commond;

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

void PlaybackTagList::dealMessage(MessageReceive *message)
{
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

void PlaybackTagList::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN:
        ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(message);
        break;
    case RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE:
        ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE(message);
        break;
    default:
        break;
    }
}

void PlaybackTagList::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(MessageReceive *message)
{
    m_pageIndex = 0;
    m_pageCount = 0;
    m_searchSid = -1;

    //
    struct resp_search_tags *tags_array = (struct resp_search_tags *)message->data;
    int count = 0;
    int allCount = 0;
    m_tagList.clear();
    if (tags_array) {
        count = message->header.size / sizeof(struct resp_search_tags);
        allCount = qMin(MAX_SEARCH_BACKUP_COUNT, tags_array->allCnt);
        m_searchSid = tags_array->sid;

        for (int i = 0; i < count; ++i) {
            m_tagList.append(tags_array[i]);
        }
    }
    qDebug() << QString("PlaybackTagList::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN, sid: %1, count: %2, all count: %3").arg(m_searchSid).arg(count).arg(allCount);

    //page
    m_pageCount = qCeil(allCount / 100.0);
    ui->lineEdit_page->setMaxPage(m_pageCount);
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(allCount).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    updateTreeList();

    if (tags_array && tags_array->allCnt >= MAX_SEARCH_BACKUP_COUNT) {
        MessageBox::queuedInformation(GET_TEXT("COMMONBACKUP/100042", "The search results is over %1, and the first %1 will be displayed.").arg(MAX_SEARCH_BACKUP_COUNT));
    }
}

void PlaybackTagList::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE(MessageReceive *message)
{
    //
    struct resp_search_tags *tags_array = (struct resp_search_tags *)message->data;
    int count = 0;
    int allCount = 0;
    m_tagList.clear();
    if (tags_array) {
        count = message->header.size / sizeof(struct resp_search_tags);
        allCount = tags_array->allCnt;

        for (int i = 0; i < count; ++i) {
            m_tagList.append(tags_array[i]);
        }
    }
    qDebug() << QString("PlaybackTagList::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE, count: %1, all count: %2").arg(count).arg(allCount);

    //page
    m_pageCount = qCeil(allCount / 100.0);
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(allCount).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    updateTreeList();
}

QStringList PlaybackTagList::treeHeaders()
{
    QStringList headerList;
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("PLAYBACK/80072", "Tag Name");
    headerList << GET_TEXT("PLAYBACK/80034", "Play");
    return headerList;
}

void PlaybackTagList::updateTreeList()
{
    ui->treeView->clearContent();
    ui->treeView->setHorizontalHeaderLabels(treeHeaders());
    ui->treeView->setRowCount(m_tagList.count());
    for (int i = 0; i < m_tagList.count(); ++i) {
        const resp_search_tags &tag = m_tagList.at(i);
        ui->treeView->setItemText(i, ColumnChannel, QString::number(tag.chnid + 1));
        ui->treeView->setItemText(i, ColumnName, QString(tag.pName));
        ui->treeView->setItemToolTip(i, ColumnName, QString(tag.pName));

        ItemKey itemKey(i, m_pageIndex);
        if (itemKey == m_currentItemKey) {
            setItemTextColor(i, "#0AA8E3");
        }
    }
}

void PlaybackTagList::playItem(int row)
{
    m_currentRow = row;
    //showWait();
    //
    if (m_currentPlaySid >= 0) {
        waitForStopAllPlayback();
        closeAllCommonPlayback();
        m_currentPlaySid = -1;
    }
    //
    const resp_search_tags &tag = m_tagList.at(row);
    int channel = tag.chnid;
    clearChannelChecked();
    setChannelChecked(channel, true);
    updateLayout(channel);
    //
    const QDateTime &startDateTime = QDateTime::fromString(tag.pTime, "yyyy-MM-dd HH:mm:ss");
    const QDateTime &endDateTime = startDateTime;
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

void PlaybackTagList::searchTagBackupPage(int page)
{
    struct rep_get_search_backup search_backup;
    memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
    search_backup.sid = m_searchSid;
    search_backup.npage = page;
    qDebug() << QString("PlaybackTagList::searchTagBackupPage, page(1-n): %1, sid: %2").arg(page).arg(m_searchSid);
    sendMessage(REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_PAGE, &search_backup, sizeof(rep_get_search_backup));
}

void PlaybackTagList::onItemClicked(const QModelIndex &index)
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

void PlaybackTagList::onItemDoubleClicked(const QModelIndex &index)
{
    playItem(index.row());
}

void PlaybackTagList::on_toolButton_firstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    searchTagBackupPage(m_pageIndex + 1);
}

void PlaybackTagList::on_toolButton_previousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    searchTagBackupPage(m_pageIndex + 1);
}

void PlaybackTagList::on_toolButton_nextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    searchTagBackupPage(m_pageIndex + 1);
}

void PlaybackTagList::on_toolButton_lastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    searchTagBackupPage(m_pageIndex + 1);
}

void PlaybackTagList::on_pushButton_go_clicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;
    searchTagBackupPage(m_pageIndex + 1);
}

void PlaybackTagList::on_toolButton_close_clicked()
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
    m_tagList.clear();
    if (m_searchSid >= 0) {
        sendMessage(REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_CLOSE, &m_searchSid, sizeof(int));
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
