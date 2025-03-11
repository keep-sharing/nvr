#include "PlaybackTag.h"
#include "ui_PlaybackTag.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "PlaybackTagList.h"
#include "PlaybackWindow.h"
#include "centralmessage.h"
#include "msuser.h"
#include <QKeyEvent>
#include <QtDebug>
#include <qmath.h>

const int ChannelRole = Qt::UserRole + 102;

PlaybackTag::PlaybackTag(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::PlaybackTag)
{
    ui->setupUi(this);

    //
    connect(ui->treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(ui->treeView, SIGNAL(enterPressed()), this, SLOT(onTreeViewEnterPressed()));

    ui->dateEdit_start->setProperty("style", "black");
    ui->dateEdit_end->setProperty("style", "black");

    //
    m_progress = new ProgressDialog(this);
    connect(m_progress, SIGNAL(sig_cancel()), this, SLOT(onSearchCanceled()));

    //
    initializeChannelTree();

    //
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_RETRIEVE, this);

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PlaybackTag::~PlaybackTag()
{
    delete ui;
}

void PlaybackTag::initializeData()
{
    if (!m_playList) {
        m_playList = new PlaybackTagList(PlaybackWindow::instance());
    }
    m_playList->initializeData();

    ui->lineEdit_tag->clear();
    const QDate &date = QDate::currentDate();
    ui->dateEdit_start->setDate(date);
    ui->dateEdit_end->setDate(date);
    ui->timeEdit_start->setTime(QTime(0, 0, 0));
    ui->timeEdit_end->setTime(QTime(23, 59, 59));

    updateChannelName();
    ui->treeView->clearCheck();
}

void PlaybackTag::closePlayback()
{
    if (m_playList) {
        m_playList->closePlayback();
    }
}

void PlaybackTag::setFocus()
{
    ui->treeView->setFocus();
    ui->treeView->setCurrentRow(0);
}

bool PlaybackTag::hasFocus()
{
    bool result = false;
    if (ui->treeView->hasFocus()) {
        result = true;
    } else if (ui->dateEdit_start->hasFocus()) {
        result = true;
    } else if (ui->timeEdit_start->hasFocus()) {
        result = true;
    } else if (ui->dateEdit_end->hasFocus()) {
        result = true;
    } else if (ui->timeEdit_end->hasFocus()) {
        result = true;
    } else if (ui->lineEdit_tag->hasFocus()) {
        result = true;
    } else if (ui->pushButton_search->hasFocus()) {
        result = true;
    }
    return result;
}

bool PlaybackTag::focusPrevious()
{
    return false;
}

bool PlaybackTag::focusNext()
{
    bool result = true;
    if (ui->pushButton_search->hasFocus()) {
        result = false;
    } else {
        focusNextChild();
    }
    return result;
}

NetworkResult PlaybackTag::dealNetworkCommond(const QString &commond)
{
    NetworkResult result = NetworkReject;
    if (commond.startsWith("Enter")) {
        if (ui->treeView->hasFocus()) {
            int row = ui->treeView->currentIndex().row();
            if (row < 0) {
                result = NetworkReject;
            } else {
                onItemClicked(row, 0);
                result = NetworkAccept;
            }
        }
    } else if (m_playList && m_playList->isVisible()) {
        result = m_playList->dealNetworkCommond(commond);
    }
    return result;
}

void PlaybackTag::dealMessage(MessageReceive *message)
{
    m_playList->dealMessage(message);
}

void PlaybackTag::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN:
        ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(message);
        message->accept();
        break;
    }
}

void PlaybackTag::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(message);
        message->accept();
        break;
    }
}

void PlaybackTag::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(MessageReceive *message)
{
    qDebug() << QString("PlaybackTag::ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN");

    m_searchSid = 0;
    m_progress->hideProgress();
    m_isSearching = false;

    //
    struct resp_search_tags *tags_array = (struct resp_search_tags *)message->data;
    int count = 0;
    if (tags_array) {
        count = tags_array->allCnt;
    }

    if (!tags_array || count == 0) {
        qDebug() << QString("PlaybackTag::ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP, no matching video files.");
        ShowMessageBox(GET_TEXT("DISK/92026", "No matching video files."));
        return;
    }

    m_playList->show();
    m_playList->processMessage(message);
}

void PlaybackTag::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message)
{
    if (!m_isSearching) {
        return;
    }

    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    if (!progressinfo) {
        qWarning() << "PlaybackTag::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, data is null.";
        return;
    }

    if (!m_progress->isVisible()) {
        m_progress->showProgress();
    }
    m_progress->setProgressValue(progressinfo->percent);
    m_searchSid = progressinfo->searchid;
}

void PlaybackTag::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "====PlaybackTag::keyPressEvent====";
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

void PlaybackTag::returnPressed()
{
    if (ui->treeView->hasFocus()) {
        int row = ui->treeView->currentIndex().row();
        if (row < 0) {

        } else {
            onItemClicked(row, 0);
        }
    }
}

void PlaybackTag::initializeChannelTree()
{
    ui->treeView->clearContent();
    //channel tree
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("COMMON/1051", "Name");
    ui->treeView->setHorizontalHeaderLabels(headerList);
    ui->treeView->setColumnWidth(0, 50);
    ui->treeView->setColumnWidth(1, 50);
    ui->treeView->setHorizontalHeaderCheckable(true);
    ui->treeView->setColumnCount(headerList.count());
    if (!m_columnCheckDelegate) {
        m_columnCheckDelegate = new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonCheckBox, this);
    }
    ui->treeView->setItemDelegateForColumn(0, m_columnCheckDelegate);
}

void PlaybackTag::updateChannelName()
{
    struct osd osdList[MAX_CAMERA];
    int osdCount = 0;
    read_osds(SQLITE_FILE_NAME, osdList, &osdCount);
    //
    QList<int> channelList = gMsUser.accessiblePlaybackChannelList();
    ui->treeView->setRowCount(channelList.size());
    int row = 0;
    for (int i = 0; i < channelList.size(); ++i) {
        int channel = channelList.at(i);
        ui->treeView->setItemData(row, 0, channel, ChannelRole);
        ui->treeView->setItemData(row, 0, false, ItemCheckedRole);
        ui->treeView->setItemText(row, 1, QString::number(channel + 1));
        ui->treeView->setItemText(row, 2, osdList[channel].name);
        row++;
    }
}

void PlaybackTag::selectNextChannel()
{
    int channel = ui->treeView->currentIndex().row();
    if (channel < 0) {
        channel = 0;
    } else if (channel < ui->treeView->rowCount() - 1) {
        channel++;
    }
    ui->treeView->setCurrentRow(channel);
}

void PlaybackTag::onLanguageChanged()
{
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("COMMON/1051", "Name");
    ui->treeView->setHorizontalHeaderLabels(headerList);
    //
    ui->label_tag->setText(GET_TEXT("PLAYBACK/80076", "Tag"));
    ui->label_startTime->setText(GET_TEXT("LOG/64001", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("LOG/64002", "End Time"));
    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
}

void PlaybackTag::onItemClicked(const QModelIndex &index)
{
    onItemClicked(index.row(), index.column());
}

void PlaybackTag::onItemClicked(int row, int column)
{
    Q_UNUSED(column)

    bool checked = ui->treeView->isItemChecked(row);
    checked = !checked;
    ui->treeView->setItemChecked(row, checked);
}

void PlaybackTag::onTreeViewEnterPressed()
{
    int row = ui->treeView->currentIndex().row();
    if (row < 0) {

    } else {
        onItemClicked(row, 0);
    }
}

void PlaybackTag::onSearchCanceled()
{
    if (m_searchSid >= 0) {
        sendMessageOnly(REQUEST_FLAG_SEARCH_TAG_PLAYBACK_CANCEL, &m_searchSid, sizeof(int));
        m_searchSid = -1;
    }
}

void PlaybackTag::on_pushButton_search_clicked()
{
    QList<int> checkedList;
    for (int i = 0; i < ui->treeView->rowCount(); ++i) {
        const bool &checked = ui->treeView->isItemChecked(i);
        if (checked) {
            int channel = ui->treeView->itemData(i, 0, ChannelRole).toInt();
            checkedList.append(channel);
        }
    }
    if (checkedList.isEmpty()) {
        ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel"));
        return;
    }

    //
    const QDateTime startDateTime(ui->dateEdit_start->date(), ui->timeEdit_start->time());
    const QDateTime endDateTime(ui->dateEdit_end->date(), ui->timeEdit_end->time());
    if (startDateTime > endDateTime) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }

    //
    struct req_search_tags search_tags;
    memset(&search_tags, 0, sizeof(struct req_search_tags));
    search_tags.chnNum = checkedList.size();
    makeChannelMask(checkedList, search_tags.chnMaskl, sizeof(search_tags.chnMaskl));
    search_tags.enType = playbackStream();
    snprintf(search_tags.name, sizeof(search_tags.name), "%s", ui->lineEdit_tag->text().toLocal8Bit().data());
    snprintf(search_tags.pStartTime, sizeof(search_tags.pStartTime), "%s", startDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(search_tags.pEndTime, sizeof(search_tags.pEndTime), "%s", endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());

    sendMessage(REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_OPEN, (void *)&search_tags, sizeof(struct req_search_tags));

    m_isSearching = true;
}
