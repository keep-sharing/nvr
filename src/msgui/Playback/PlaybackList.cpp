#include "PlaybackList.h"
#include "ui_PlaybackList.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "PlaybackWindow.h"
#include "centralmessage.h"
#include <QDesktopWidget>
#include <QtDebug>

QDateTime PlaybackList::s_beginDateTime;
QDateTime PlaybackList::s_endDateTime;

PlaybackList::PlaybackList(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::PlaybackList)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);

    m_channelTextDelegate = new ItemTextDelegate(this);
    m_timeTextDelegate = new ItemTextDelegate(this);

    //
    ui->comboBox_prePlayback->setItemData(0, 5);
    ui->comboBox_prePlayback->setItemData(1, 10);
    ui->comboBox_prePlayback->setItemData(2, 30);
    ui->comboBox_prePlayback->setItemData(3, 60);
    ui->comboBox_prePlayback->setItemData(4, 120);
    ui->comboBox_prePlayback->setItemData(5, 300);
    ui->comboBox_prePlayback->setItemData(6, 600);

    ui->comboBox_postPlayback->setItemData(0, 5);
    ui->comboBox_postPlayback->setItemData(1, 10);
    ui->comboBox_postPlayback->setItemData(2, 30);
    ui->comboBox_postPlayback->setItemData(3, 60);
    ui->comboBox_postPlayback->setItemData(4, 120);
    ui->comboBox_postPlayback->setItemData(5, 300);
    ui->comboBox_postPlayback->setItemData(6, 600);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PlaybackList::~PlaybackList()
{
    delete ui;
}

void PlaybackList::initializeData()
{
    ui->comboBox_prePlayback->setCurrentIndexFromData(30);
    ui->comboBox_postPlayback->setCurrentIndexFromData(30);
}

void PlaybackList::dealMessage(MessageReceive *message)
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

void PlaybackList::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(MessageReceive *message)
{
    resp_search_common_backup *common_backup_array = (resp_search_common_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_common_backup);

    if (!common_backup_array) {
        qWarning() << QString("PlaybackList::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN, resp_search_common_backup is null.");
        //
        //closeWait();
        ShowMessageBox(GET_TEXT("PLAYBACK/80091", "The video file may have expired or been overwritten."));
    } else {
        if (common_backup_array->allCnt == 0) {
            qDebug() << QString("PlaybackList::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN, resp_search_common_backup is empty.");
            //
            //closeWait();
            ShowMessageBox(GET_TEXT("PLAYBACK/80091", "The video file may have expired or been overwritten."));
        } else {
            const int channel = common_backup_array->chnid;
            m_currentPlaySid = common_backup_array->sid;

            PlaybackChannelInfo &channelInfo = s_channelInfoMap[channel];
            channelInfo.setCommonBackup(common_backup_array, count);
        }
    }
    exitWait();
}

void PlaybackList::ON_RESPONSE_FLAG_START_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)

    exitWait();
}

void PlaybackList::ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)

    qDebug() << "PlaybackList::ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK";
    exitWait();
}

void PlaybackList::showEvent(QShowEvent *event)
{
    m_currentItemKey = ItemKey();

    QRect screenRect = qApp->desktop()->screenGeometry();
    int rightWidth = screenRect.width() * 288 / 1920.0;
    setGeometry(screenRect.right() - rightWidth + 1, screenRect.top(), rightWidth, screenRect.height());

    QWidget::showEvent(event);
}

void PlaybackList::initializeTreeView()
{
    const QStringList &headerList = treeHeaders();
    ui->treeView->setHorizontalHeaderLabels(headerList);
    ui->treeView->setColumnCount(3);
    ui->treeView->setColumnWidth(ColumnChannel, 30);
    ui->treeView->setColumnWidth(ColumnTime, 180);
    ui->treeView->setItemDelegateForColumn(ColumnChannel, m_channelTextDelegate);
    ui->treeView->setItemDelegateForColumn(ColumnTime, m_timeTextDelegate);
    ui->treeView->setItemDelegateForColumn(ColumnPlay, new ItemButtonDelegate(QPixmap(":/playback/playback/event_play.png"), QSize(18, 18), this));
    connect(ui->treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));
}

void PlaybackList::updateTreeList()
{
}

void PlaybackList::playItem(int row)
{
    Q_UNUSED(row)
}

void PlaybackList::waitForSearchCommonPlayback(int channel, const QDateTime &begin, const QDateTime &end)
{
    long pretime = ui->comboBox_prePlayback->currentData().toInt();
    long posttime = ui->comboBox_postPlayback->currentData().toInt();

    s_beginDateTime = begin.addSecs(0 - pretime);
    if (s_beginDateTime.date() != begin.date()) {
        s_beginDateTime.setDate(begin.date());
        s_beginDateTime.setTime(QTime(0, 0));
    }
    s_endDateTime = end.addSecs(posttime);
    if (s_endDateTime.date() != end.date()) {
        s_endDateTime.setDate(end.date());
        s_endDateTime.setTime(QTime(23, 59, 59));
    }

    //
    req_search_common_backup common_backup;
    memset(&common_backup, 0, sizeof(req_search_common_backup));

    makeChannelMask(channel, common_backup.chnMaskl, sizeof(common_backup.chnMaskl));
    common_backup.chnNum = 1;
    common_backup.enType = playbackStream();
    common_backup.enEvent = REC_EVENT_ALL;
    common_backup.enState = SEG_STATE_ALL;
    snprintf(common_backup.pStartTime, sizeof(common_backup.pStartTime), "%s", s_beginDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(common_backup.pEndTime, sizeof(common_backup.pEndTime), "%s", s_endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());

    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_COM_PLAYBACK_OPEN, (void *)&common_backup, sizeof(req_search_common_backup));
    execWait();
}

void PlaybackList::waitForStartAllPlayback()
{
    rep_start_all_play start_all;
    memset(&start_all, 0, sizeof(rep_start_all_play));
    start_all.actState = PB_PLAY;
    start_all.actSpeed = playbackSpeed();
    start_all.actdir = playbackDirection();
    makeChannelMask(start_all.chnMaskl, sizeof(start_all.chnMaskl));

    snprintf(start_all.pPlayTime, sizeof(start_all.pPlayTime), "%s", s_beginDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(start_all.pStartTime, sizeof(start_all.pStartTime), "%s", s_beginDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(start_all.pEndTime, sizeof(start_all.pEndTime), "%s", s_endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());

    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_START_ALL_PLAYBACK, (void *)&start_all, sizeof(rep_start_all_play));
    execWait();
}

void PlaybackList::setItemTextColor(int row, const QString &color)
{
    ui->treeView->setItemData(row, ColumnChannel, color, ItemTextDelegate::TextColorRole);
    ui->treeView->setItemData(row, ColumnTime, color, ItemTextDelegate::TextColorRole);
}

void PlaybackList::clearItemTextColor(int row)
{
    setItemTextColor(row, QString());
}

void PlaybackList::clearAllItemTextColor()
{
    for (int i = 0; i < ui->treeView->rowCount(); ++i) {
        clearItemTextColor(i);
    }
}

void PlaybackList::onLanguageChanged()
{
    ui->label_searchResult->setText(GET_TEXT("PLAYBACK/80071", "Search Result:"));

    ui->label_prePlayback->setText(GET_TEXT("EVENTBACKUP/101002", "Pre Playback"));
    ui->label_postPlayback->setText(GET_TEXT("EVENTBACKUP/101003", "Post Playback"));

    ui->toolButton_firstPage->setToolTip(GET_TEXT("PLAYBACK/80061", "First Page"));
    ui->toolButton_previousPage->setToolTip(GET_TEXT("PLAYBACK/80062", "Previous Page"));
    ui->toolButton_nextPage->setToolTip(GET_TEXT("PLAYBACK/80063", "Next Page"));
    ui->toolButton_lastPage->setToolTip(GET_TEXT("PLAYBACK/80064", "Last Page"));
    ui->pushButton_go->setText(GET_TEXT("PLAYBACK/80065", "Go"));

    ui->pushButton_export->setText(GET_TEXT("LOG/64013", "Export"));
    ui->pushButton_exportAll->setText(GET_TEXT("PLAYBACK/80073", "Export All"));
}

void PlaybackList::onItemClicked(const QModelIndex &index)
{
    Q_UNUSED(index)
}

void PlaybackList::onItemDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index)
}

void PlaybackList::selectPrevious()
{
    if (m_currentRow <= 0) {
        m_currentRow = 0;
    } else {
        m_currentRow--;
    }
    ui->treeView->setCurrentRow(m_currentRow);
}

void PlaybackList::selectNext()
{
    if (m_currentRow >= ui->treeView->rowCount() - 1) {
        m_currentRow = ui->treeView->rowCount() - 1;
    } else {
        m_currentRow++;
    }
    ui->treeView->setCurrentRow(m_currentRow);
}

void PlaybackList::on_toolButton_firstPage_clicked()
{
}

void PlaybackList::on_toolButton_previousPage_clicked()
{
}

void PlaybackList::on_toolButton_nextPage_clicked()
{
}

void PlaybackList::on_toolButton_lastPage_clicked()
{
}

void PlaybackList::on_pushButton_go_clicked()
{
}

void PlaybackList::on_comboBox_prePlayback_activated(int index)
{
    Q_UNUSED(index)

    if (m_currentRow < 0) {
        return;
    }
    playItem(m_currentRow);
}

void PlaybackList::on_comboBox_postPlayback_activated(int index)
{
    Q_UNUSED(index)

    if (m_currentRow < 0) {
        return;
    }
    playItem(m_currentRow);
}

void PlaybackList::on_pushButton_export_clicked()
{
}

void PlaybackList::on_pushButton_exportAll_clicked()
{
}

void PlaybackList::on_toolButton_close_clicked()
{
}
