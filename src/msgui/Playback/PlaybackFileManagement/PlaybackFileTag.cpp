#include "PlaybackFileTag.h"
#include "ui_AbstractPlaybackFile.h"
#include "AnimateToast.h"
#include "BasePlayback.h"
#include "CustomTag.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "PlaybackBar.h"
#include "PlaybackSplit.h"
#include "centralmessage.h"
#include <qmath.h>
#include "PlaybackTagData.h"

struct CommonTagFileInfo {
    int channel = 0;
    int sid = 0;
    QString dateTime;
};

PlaybackFileTag::PlaybackFileTag(QWidget *parent)
    : AbstractPlaybackFile(parent)
{
    m_waitting = new MsWaitting(this);

    initializeTableView();

    ui->label_selectedCount->hide();
    ui->label_selectedSize->hide();
}

void PlaybackFileTag::getTags()
{
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_CLEAR_TEMP_TAGS_PLAYBACK, NULL, 0);
    m_pageIndex = 0;
    //
    m_searchPage = 1;
    struct req_search_tags search_tags;
    memset(&search_tags, 0, sizeof(struct req_search_tags));
    search_tags.chnNum = 1;
    if (BasePlayback::playbackType() == SplitPlayback) {
        if (PlaybackSplit::instance()->isPlaying()) {
            int channel = PlaybackSplit::instance()->channel();
            BasePlayback::makeChannelMask(channel, search_tags.chnMaskl, sizeof(search_tags.chnMaskl));
        }
    } else {
        BasePlayback::makeChannelMask(BasePlayback::playingChannels(), search_tags.chnMaskl, sizeof(search_tags.chnMaskl));
    }
    search_tags.enType = BasePlayback::playbackStream();
    snprintf(search_tags.pStartTime, sizeof(search_tags.pStartTime), "%s 00:00:00", BasePlayback::playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    snprintf(search_tags.pEndTime, sizeof(search_tags.pEndTime), "%s 23:59:59", BasePlayback::playbackDate().toString("yyyy-MM-dd").toStdString().c_str());
    qMsDebug() << QString("REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_OPEN, chnMaskl:%1, entype:%2, pStartTime:%3, pEndTime:%4")
                  .arg(search_tags.chnMaskl).arg(search_tags.enType).arg(search_tags.pStartTime).arg(search_tags.pEndTime);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_OPEN, (void *)&search_tags, sizeof(struct req_search_tags));
    //m_waitting->//showWait();
}

void PlaybackFileTag::updateTableList()
{
    AbstractPlaybackFile::updateTableList();

    ui->tableView->clearContent();
    int row = 0;
    for (auto iter = m_fileMap.constBegin(); iter != m_fileMap.constEnd(); ++iter) {
        const auto &key = iter.key();
        const auto &value = iter.value();
        ui->tableView->setItemIntValue(row, ColumnChannel, value.chnid + 1);
        ui->tableView->setItemData(row, ColumnChannel, QVariant::fromValue(value), FileInfoRole);
        ui->tableView->setItemText(row, ColumnName, value.pName);
        ui->tableView->setItemText(row, ColumnTime, key.startDateTimeString());

        row++;
    }
}

void PlaybackFileTag::clear()
{
    m_fileMap.clear();
    ui->tableView->clearContent();

    ui->label_selectedCount->setText(QString("%1 %2").arg(GET_TEXT("PLAYBACK/80106", "Selected Items:")).arg(0));
    ui->label_selectedSize->setText(QString("%1 %2").arg(GET_TEXT("PLAYBACK/80107", "Total Size:")).arg(TableView::bytesString(0)));
}

int PlaybackFileTag::waitForTagAll(const QMap<int, QDateTime> &dateTimeMap, const QString &name)
{
    m_tagSuccessCount = 0;

    QList<CommonTagFileInfo> infoList;
    if (BasePlayback::playbackType() == SplitPlayback) {
        for (auto iter = dateTimeMap.constBegin(); iter != dateTimeMap.constEnd(); ++iter) {
            int channel = PlaybackSplit::instance()->channel();
            int sid = iter.key();
            QDateTime dateTime = iter.value();
            const resp_search_common_backup &search_backup = PlaybackSplit::instance()->findCommonBackup(sid, dateTime);
            if (search_backup.chnid < 0) {
                AnimateToast::s_animateToast->addItem(QString("%1-%2").arg(channel + 1).arg(sid + 1), GET_TEXT("PLAYBACK/80115", "No record"));
                continue;
            }
            CommonTagFileInfo info;
            info.channel = channel;
            info.sid = search_backup.sid;
            info.dateTime = dateTime.toString("yyyy-MM-dd HH:mm:ss");
            infoList.append(info);
        }
    } else {
        const QList<int> &channelList = BasePlayback::channelCheckedList();
        for (int i = 0; i < channelList.size(); ++i) {
            int channel = channelList.at(i);
            QDateTime dateTime = dateTimeMap.value(0);
            const resp_search_common_backup &search_backup = BasePlayback::findCommonBackup(channel, dateTime);
            if (search_backup.chnid < 0) {
                AnimateToast::s_animateToast->addItem(QString::number(channel + 1), GET_TEXT("PLAYBACK/80115", "No record"));
                continue;
            }
            CommonTagFileInfo info;
            info.channel = channel;
            info.sid = search_backup.sid;
            info.dateTime = dateTime.toString("yyyy-MM-dd HH:mm:ss");
            infoList.append(info);
        }
    }

    //
    for (int i = 0; i < infoList.size(); ++i) {
        const CommonTagFileInfo &info = infoList.at(i);

        //
        rep_set_tags_record tags_record;
        memset(&tags_record, 0, sizeof(struct rep_set_tags_record));
        tags_record.chnid = info.channel;
        tags_record.sid = info.sid;
        snprintf(tags_record.pName, sizeof(tags_record.pName), "%s", name.toStdString().c_str());
        snprintf(tags_record.pTime, sizeof(tags_record.pTime), "%s", info.dateTime.toStdString().c_str());
        qDebug() << "REQUEST_FLAG_SET_TEMP_TAGS_PLAYBACK"
                 << ", channel:" << tags_record.chnid
                 << ", sid:" << tags_record.sid
                 << ", pName:" << tags_record.pName
                 << ", pTime:" << tags_record.pTime;
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_TEMP_TAGS_PLAYBACK, (void *)&tags_record, sizeof(rep_set_tags_record));

        // int result = m_eventLoop.exec();

        // //
        // if (result == 0) {
        //     QDateTime dateTime = QDateTime::fromString(info.dateTime, "yyyy-MM-dd HH:mm:ss");
        //     gPlaybackTagData.addTag(info.channel, dateTime.toTime_t());
        // }
    }
    return m_tagSuccessCount;
}

int PlaybackFileTag::waitForTag(const QDateTime &dateTime, const QString &name)
{
    m_tagSuccessCount = 0;

    QList<CommonTagFileInfo> infoList;
    if (BasePlayback::playbackType() == SplitPlayback) {
        int channel = PlaybackSplit::instance()->channel();
        int sid = PlaybackSplit::instance()->selectedSid();
        const resp_search_common_backup &search_backup = PlaybackSplit::instance()->findCommonBackup(sid, dateTime);
        if (search_backup.chnid < 0) {
            AnimateToast::s_animateToast->addItem(QString("%1-%2").arg(channel + 1).arg(sid + 1), GET_TEXT("PLAYBACK/80115", "No record"));
        } else {
            CommonTagFileInfo info;
            info.channel = channel;
            info.sid = search_backup.sid;
            info.dateTime = dateTime.toString("yyyy-MM-dd HH:mm:ss");
            infoList.append(info);
        }
    } else {
        int channel = BasePlayback::currentChannel();
        const resp_search_common_backup &search_backup = BasePlayback::findCommonBackup(channel, dateTime);
        if (search_backup.chnid < 0) {
            AnimateToast::s_animateToast->addItem(QString::number(channel + 1), GET_TEXT("PLAYBACK/80115", "No record"));
        } else {
            CommonTagFileInfo info;
            info.channel = channel;
            info.sid = search_backup.sid;
            info.dateTime = dateTime.toString("yyyy-MM-dd HH:mm:ss");
            infoList.append(info);
        }
    }

    //
    for (int i = 0; i < infoList.size(); ++i) {
        const CommonTagFileInfo &info = infoList.at(i);

        //
        rep_set_tags_record tags_record;
        memset(&tags_record, 0, sizeof(struct rep_set_tags_record));
        tags_record.chnid = info.channel;
        tags_record.sid = info.sid;
        snprintf(tags_record.pName, sizeof(tags_record.pName), "%s", name.toStdString().c_str());
        snprintf(tags_record.pTime, sizeof(tags_record.pTime), "%s", info.dateTime.toStdString().c_str());
        qDebug() << "REQUEST_FLAG_SET_TEMP_TAGS_PLAYBACK"
                 << ", channel:" << tags_record.chnid
                 << ", sid:" << tags_record.sid
                 << ", pName:" << tags_record.pName
                 << ", pTime:" << tags_record.pTime;
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_TEMP_TAGS_PLAYBACK, (void *)&tags_record, sizeof(rep_set_tags_record));

        // int result = m_eventLoop.exec();

        // //
        // if (result == 0) {
        //     gPlaybackTagData.addTag(info.channel, dateTime.toTime_t());
        // }
    }
    return m_tagSuccessCount;
}

void PlaybackFileTag::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN:
        if (m_waitting->isVisible()) {
            ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(message);
            message->accept();
        }
        break;
    case RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE:
        if (m_waitting->isVisible()) {
            ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE(message);
            message->accept();
        }
        break;
    case RESPONSE_FLAG_SET_TEMP_TAGS_PLAYBACK:
        ON_RESPONSE_FLAG_SET_TEMP_TAGS_PLAYBACK(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_TEMP_TAGS_PLAYBACK:
        ON_RESPONSE_FLAG_GET_TEMP_TAGS_PLAYBACK(message);
        message->accept();
        break;
    case RESPONSE_FLAG_EDIT_TEMP_TAGS_PLAYBACK:
        ON_RESPONSE_FLAG_EDIT_TEMP_TAGS_PLAYBACK(message);
        message->accept();
        break;
    case RESPONSE_FLAG_REMOVE_TEMP_TAGS_PLAYBACK:
        ON_RESPONSE_FLAG_REMOVE_TEMP_TAGS_PLAYBACK(message);
        message->accept();
        break;
    default:
        break;
    }
}

void PlaybackFileTag::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(MessageReceive *message)
{
    struct resp_search_tags *tags_array = (struct resp_search_tags *)message->data;
    if (tags_array) {
        qMsDebug() << QString("sid: %1, allCnt: %2").arg(tags_array->sid).arg(tags_array->allCnt);
        m_searchPageCount = qCeil(tags_array->allCnt / 100.0);
        if (m_searchPage < m_searchPageCount) {
            m_searchPage++;
            struct rep_get_search_backup search_backup;
            memset(&search_backup, 0, sizeof(struct rep_get_search_backup));
            search_backup.sid = tags_array->sid;
            search_backup.npage = m_searchPage;
            MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_PAGE, &search_backup, sizeof(rep_get_search_backup));
            return;
        }
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_CLOSE, &tags_array->sid, sizeof(tags_array->sid));
    }
    //m_waitting->//closeWait();
    getTempTags(1);
}

void PlaybackFileTag::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE(MessageReceive *message)
{
    ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(message);
}

void PlaybackFileTag::ON_RESPONSE_FLAG_SET_TEMP_TAGS_PLAYBACK(MessageReceive *message)
{
    int result = -1;

    qDebug() << "====PlaybackFileTag::ON_RESPONSE_FLAG_SET_TEMP_TAGS_PLAYBACK====";

    resp_common_state *stateList = (resp_common_state *)message->data;
    if (!stateList) {
        qDebug() << "----data:" << stateList;
    } else {
        int count = message->header.size / sizeof(resp_common_state);
        for (int i = 0; i < count; ++i) {
            const resp_common_state &state = stateList[i];
            qDebug() << "----chnid:" << state.chnid << ", state:" << state.state;
            if (state.chnid < 0) {
                continue;
            }
            result = state.state;
            if (state.state != MF_SUCCESS) {
                QString errStr;
                switch (state.state) {
                case PB_ERR_UNKNOWN:
                    errStr = GET_TEXT("PLAYBACK/80102", "Add tag failed");
                    break;
                case PB_ERR_TAG_OVERFLOW:
                    errStr = GET_TEXT("PLAYBACK/80103", "Add tag failed, the max. tag number of each video file is 64.");
                    break;
                case PB_ERR_STREAM_INVALID:
                    errStr = GET_TEXT("PLAYBACK/80102", "Add tag failed");
                    break;
                default:
                    errStr = GET_TEXT("PLAYBACK/80102", "Add tag failed");
                    break;
                }

                AnimateToast::s_animateToast->addItem(QString::number(state.chnid + 1), errStr);
            } else {
                m_tagSuccessCount++;
            }
        }
    }
    m_eventLoop.exit(result);
}

void PlaybackFileTag::ON_RESPONSE_FLAG_GET_TEMP_TAGS_PLAYBACK(MessageReceive *message)
{
    m_fileMap.clear();

    resp_search_tags *search_tags_array = (resp_search_tags *)message->data;
    int count = 0;
    if (search_tags_array) {
        count = message->header.size / sizeof(struct resp_search_tags);
        m_tagCount = search_tags_array->allCnt;

        for (int i = 0; i < count; i++) {
            const resp_search_tags &search_tags = search_tags_array[i];
            PlaybackFileKey fileKey(search_tags);
            m_fileMap.insert(fileKey, search_tags);
        }
    } else {
        m_tagCount = 0;
    }

    qDebug() << QString("PlaybackFileTag::ON_RESPONSE_FLAG_GET_TEMP_TAGS_PLAYBACK, count: %1, all count: %2").arg(count).arg(m_tagCount);

    updateTableList();
}

void PlaybackFileTag::ON_RESPONSE_FLAG_EDIT_TEMP_TAGS_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)

    //m_waitting->//closeWait();
}

void PlaybackFileTag::ON_RESPONSE_FLAG_REMOVE_TEMP_TAGS_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)

    //m_waitting->//closeWait();
}

int PlaybackFileTag::pageCount()
{
    m_pageCount = qCeil(m_tagCount / 100.0);
    ui->lineEdit_page->setMaxPage(m_pageCount);
    return m_pageCount;
}

int PlaybackFileTag::itemCount()
{
    return m_tagCount;
}

QStringList PlaybackFileTag::tableHeaders()
{
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("PLAYBACK/80072", "Tag Name");
    headerList << GET_TEXT("LOG/64005", "Time");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    return headerList;
}

void PlaybackFileTag::initializeTableView()
{
    const QStringList &headerList = tableHeaders();
    ui->tableView->setHorizontalHeaderLabels(headerList);
    //ui->tableView->setHeaderCheckable(false);
    ui->tableView->setColumnCount(headerList.size());
    //ui->tableView->hideColumn(ColumnCheck);
    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //sort
    ui->tableView->setSortType(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortableForColumn(ColumnEdit, false);
    ui->tableView->setSortableForColumn(ColumnDelete, false);
    //column width
    ui->tableView->setColumnWidth(ColumnChannel, 100);
    ui->tableView->setColumnWidth(ColumnName, 100);
    ui->tableView->setColumnWidth(ColumnTime, 400);
    ui->tableView->setColumnWidth(ColumnEdit, 100);
}

/**
 * @brief PlaybackFileTag::getTempTags
 * @param page: 1-n
 */
void PlaybackFileTag::getTempTags(int page)
{
    qDebug() << QString("PlaybackFileTag::getTempTags, page(1-n): %1").arg(page);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_TEMP_TAGS_PLAYBACK, (void *)&page, sizeof(int));
}

void PlaybackFileTag::onItemClicked(int row, int column)
{
    switch (column) {
    case ColumnEdit: {
        const resp_search_tags &search_tags = ui->tableView->itemData(row, ColumnChannel, FileInfoRole).value<resp_search_tags>();

        CustomTag customTag(this);
        customTag.editTag(search_tags.pName);
        int result = customTag.exec();
        if (result == CustomTag::Accepted) {
            rep_set_tags_record set_tags_record;
            memset(&set_tags_record, 0, sizeof(rep_set_tags_record));
            set_tags_record.chnid = search_tags.chnid;
            set_tags_record.sid = search_tags.sid;
            set_tags_record.index = search_tags.index;
            snprintf(set_tags_record.pName, sizeof(set_tags_record.pName), "%s", customTag.tagName().toStdString().c_str());
            strcpy(set_tags_record.pTime, search_tags.pTime);
            MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_EDIT_TEMP_TAGS_PLAYBACK, (void *)&set_tags_record, sizeof(rep_set_tags_record));
            m_waitting->execWait();
            PlaybackFileKey fileKey(search_tags);
            if (m_fileMap.contains(fileKey)) {
                resp_search_tags &temp_tags = m_fileMap[fileKey];
                strcpy(temp_tags.pName, set_tags_record.pName);
                updateTableList();
            }
        }
        break;
    }
    case ColumnDelete: {
        const resp_search_tags &search_tags = ui->tableView->itemData(row, ColumnChannel, FileInfoRole).value<resp_search_tags>();
        int result = MessageBox::question(this, GET_TEXT("PLAYBACK/80087", "Are you sure to delete this tag?"));
        if (result == MessageBox::Yes) {
            rep_set_tags_record set_tags_record;
            memset(&set_tags_record, 0, sizeof(rep_set_tags_record));
            set_tags_record.chnid = search_tags.chnid;
            set_tags_record.sid = search_tags.sid;
            set_tags_record.index = search_tags.index;
            strcpy(set_tags_record.pTime, search_tags.pTime);
            MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_REMOVE_TEMP_TAGS_PLAYBACK, (void *)&set_tags_record, sizeof(rep_set_tags_record));
            m_waitting->execWait();
            PlaybackFileKey fileKey(search_tags);
            m_fileMap.remove(fileKey);
            m_tagCount--;
            updateTableList();

            //
            gPlaybackTagData.removeTag(search_tags.chnid, QDateTime::fromString(QString(search_tags.pTime), "yyyy-MM-dd HH:mm:ss").toTime_t());
        }
        break;
    }
    default:
        break;
    }
}

void PlaybackFileTag::on_toolButton_firstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    getTempTags(m_pageIndex + 1);
}

void PlaybackFileTag::on_toolButton_previousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    getTempTags(m_pageIndex + 1);
}

void PlaybackFileTag::on_toolButton_nextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    getTempTags(m_pageIndex + 1);
}

void PlaybackFileTag::on_toolButton_lastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    getTempTags(m_pageIndex + 1);
}

void PlaybackFileTag::on_pushButton_go_clicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;
    getTempTags(m_pageIndex + 1);
}

void PlaybackFileTag::deleteTag()
{
    if (!ui->tableView->rowCount())
        return;

    bool hasSelected = false;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (ui->tableView->isItemChecked(row)) {
            hasSelected = true;
            break;
        }
    }
    if (!hasSelected) {
        ShowMessageBox(GET_TEXT("PLAYBACK/80120", "please select at least one tag."));
        return;
    }
    int result = MessageBox::question(this, GET_TEXT("PLAYBACK/80121", "Are you sure to delete selected tags?"));
    if (result == MessageBox::Cancel) {
        return;
    }

    //
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (!ui->tableView->isItemChecked(row)) {
            continue;
        }

        const resp_search_tags &search_tags = ui->tableView->itemData(row, ColumnChannel, FileInfoRole).value<resp_search_tags>();
        rep_set_tags_record set_tags_record;
        memset(&set_tags_record, 0, sizeof(rep_set_tags_record));
        set_tags_record.chnid = search_tags.chnid;
        set_tags_record.sid = search_tags.sid;
        set_tags_record.index = search_tags.index;
        strcpy(set_tags_record.pTime, search_tags.pTime);
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_REMOVE_TEMP_TAGS_PLAYBACK, (void *)&set_tags_record, sizeof(rep_set_tags_record));
        m_waitting->execWait();
        PlaybackFileKey fileKey(search_tags);
        m_fileMap.remove(fileKey);
        m_tagCount--;
        //
        gPlaybackTagData.removeTag(search_tags.chnid, QDateTime::fromString(QString(search_tags.pTime), "yyyy-MM-dd HH:mm:ss").toTime_t());
    }

    updateTableList();
}

void PlaybackFileTag::showEvent(QShowEvent *event)
{
    AbstractPlaybackFile::showEvent(event);
    //
    QTimer::singleShot(100, this, SLOT(getTags()));
}

void PlaybackFileTag::onLanguageChanged()
{
    AbstractPlaybackFile::onLanguageChanged();

    ui->tableView->setHorizontalHeaderLabels(tableHeaders());
}

void PlaybackFileTag::deleteAllTag()
{
    if (!ui->tableView->rowCount())
        return;

    int result = MessageBox::question(this, GET_TEXT("PLAYBACK/80122", "Are you sure to delete all tags?"));
    if (result == MessageBox::Cancel) {
        return;
    }

    for (auto iter = m_fileMap.constBegin(); iter != m_fileMap.constEnd(); ++iter) {
        const resp_search_tags &search_tags = iter.value();

        rep_set_tags_record set_tags_record;
        memset(&set_tags_record, 0, sizeof(rep_set_tags_record));
        set_tags_record.chnid = search_tags.chnid;
        set_tags_record.sid = search_tags.sid;
        set_tags_record.index = search_tags.index;
        strcpy(set_tags_record.pTime, search_tags.pTime);
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_REMOVE_TEMP_TAGS_PLAYBACK, (void *)&set_tags_record, sizeof(rep_set_tags_record));
        m_waitting->execWait();
        //
        gPlaybackTagData.removeTag(search_tags.chnid, QDateTime::fromString(QString(search_tags.pTime), "yyyy-MM-dd HH:mm:ss").toTime_t());
    }
    m_fileMap.clear();
    m_tagCount = 0;

    updateTableList();
}
