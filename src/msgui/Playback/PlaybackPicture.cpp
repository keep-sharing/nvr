#include "PlaybackPicture.h"
#include "ui_PlaybackPicture.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "PlaybackPictureList.h"
#include "PlaybackWindow.h"
#include "centralmessage.h"
#include "msuser.h"
#include <QKeyEvent>
#include <QtDebug>
#include <qmath.h>

const int AlarmKeyRole = Qt::UserRole + 101;
const int ChannelRole = Qt::UserRole + 102;

PlaybackPicture::PlaybackPicture(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::PlaybackPicture)
{
    ui->setupUi(this);

    //
    ui->comboBox_mainType->clear();
    ui->comboBox_mainType->addTranslatableItem("PICTUREBACKUP/102000", "Live View Snapshot", INFO_MAJOR_LIVE_PIC);
    ui->comboBox_mainType->addTranslatableItem("PICTUREBACKUP/102001", "Playback Snapshot", INFO_MAJOR_PB_PIC);
    if (qMsNvr->isSupportContinuousSnapshot()) {
        ui->comboBox_mainType->addTranslatableItem("PICTUREBACKUP/102006", "Continuous Snapshot", INFO_MAJOR_TIME_PIC);
    }
    if (qMsNvr->isSupportEventSnapshot()) {
        ui->comboBox_mainType->addTranslatableItem("PICTUREBACKUP/102007", "Event Snapshot", INOF_MAJOR_EVENT_PIC);
    }

    ui->comboBox_subType->clear();
    ui->comboBox_subType->addTranslatableItem("COMMON/1006", "All", INFO_MINOR_PIC_EVENT);
    ui->comboBox_subType->addTranslatableItem("MOTION/51000", "Motion Detection", INFO_MINOR_PIC_MOTION);
    ui->comboBox_subType->addTranslatableItem("AUDIO_ALARM/159000", "Audio Alarm", INFO_MINOR_PIC_AUDIO_ALARM);
    ui->comboBox_subType->addTranslatableItem("ALARMIN/52001", "Alarm Input", INFO_MINOR_PIC_ALARM);
    ui->comboBox_subType->addTranslatableItem("SMARTEVENT/55000", "VCA", INFO_MINOR_PIC_VCA);
    if (!qMsNvr->is3536c()) {
        ui->comboBox_subType->addTranslatableItem("ANPR/103054", "Smart Analysis", INFO_MINOR_PIC_LPR);
    }

    ui->label_subType->hide();
    ui->comboBox_subType->hide();
    ui->label_tertiaryType->hide();
    ui->comboBox_tertiaryType->hide();
    //
    connect(ui->treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(ui->treeView, SIGNAL(enterPressed()), this, SLOT(onTreeViewEnterPressed()));

    ui->dateEdit_start->setProperty("style", "black");
    ui->dateEdit_end->setProperty("style", "black");

    //
    m_progress = new ProgressDialog(this);
    connect(m_progress, SIGNAL(sig_cancel()), this, SLOT(onSearchCanceled()));

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_RETRIEVE, this);

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PlaybackPicture::~PlaybackPicture()
{
    delete ui;
}

void PlaybackPicture::initializeData()
{
    if (!m_playList) {
        m_playList = new PlaybackPictureList(PlaybackWindow::instance());
    }
    m_playList->initializeData();

    const QDate &date = QDate::currentDate();
    ui->dateEdit_start->setDate(date);
    ui->dateEdit_end->setDate(date);
    ui->timeEdit_start->setTime(QTime(0, 0, 0));
    ui->timeEdit_end->setTime(QTime(23, 59, 59));

    ui->comboBox_mainType->setCurrentIndex(0);
    on_comboBox_mainType_activated(0);

    initializeChannelTree();
    ui->treeView->clearCheck();
}

void PlaybackPicture::closePlayback()
{
    if (m_playList) {
        m_playList->closePlayback();
    }
}

void PlaybackPicture::setFocus()
{
    ui->treeView->setFocus();
    ui->treeView->setCurrentRow(0);
}

bool PlaybackPicture::hasFocus()
{
    bool result = false;
    if (ui->comboBox_mainType->hasFocus()) {
        result = true;
    } else if (ui->treeView->hasFocus()) {
        result = true;
    } else if (ui->dateEdit_start->hasFocus()) {
        result = true;
    } else if (ui->timeEdit_start->hasFocus()) {
        result = true;
    } else if (ui->dateEdit_end->hasFocus()) {
        result = true;
    } else if (ui->timeEdit_end->hasFocus()) {
        result = true;
    } else if (ui->pushButton_search->hasFocus()) {
        result = true;
    }
    return result;
}

bool PlaybackPicture::focusPrevious()
{
    return false;
}

bool PlaybackPicture::focusNext()
{
    bool result = true;
    if (ui->pushButton_search->hasFocus()) {
        result = false;
    } else {
        focusNextChild();
    }
    return result;
}

NetworkResult PlaybackPicture::dealNetworkCommond(const QString &commond)
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

void PlaybackPicture::dealMessage(MessageReceive *message)
{
    m_playList->dealMessage(message);
}

void PlaybackPicture::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_PIC_BACKUP:
        ON_RESPONSE_FLAG_SEARCH_PIC_BACKUP(message);
        break;
    }
}

void PlaybackPicture::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(message);
        break;
    }
}

void PlaybackPicture::ON_RESPONSE_FLAG_SEARCH_PIC_BACKUP(MessageReceive *message)
{
    m_searchSid = 0;
    m_progress->hideProgress();
    m_isSearching = false;

    //
    struct resp_search_picture_backup *picture_backup_array = (struct resp_search_picture_backup *)message->data;
    int count = message->header.size / sizeof(struct resp_search_picture_backup);

    if (!picture_backup_array || count == 0) {
        ShowMessageBox(GET_TEXT("PICTUREBACKUP/102002", "No matching picture files."));
        return;
    }

    if (picture_backup_array->allCnt >= MAX_SEARCH_BACKUP_COUNT) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100042", "The search results is over %1, and the first %1 will be displayed.").arg(MAX_SEARCH_BACKUP_COUNT));
    }

    m_playList->show();
    m_playList->processMessage(message);
}

void PlaybackPicture::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message)
{
    if (!m_isSearching) {
        return;
    }

    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    if (!progressinfo) {
        qWarning() << "PlaybackPicture::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE, data is null.";
        return;
    }

    if (!m_progress->isVisible()) {
        m_progress->showProgress();
    }
    m_progress->setProgressValue(progressinfo->percent);
    m_searchSid = progressinfo->searchid;
}

void PlaybackPicture::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "====PlaybackPicture::keyPressEvent====";
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

void PlaybackPicture::returnPressed()
{
    if (ui->treeView->hasFocus()) {
        int row = ui->treeView->currentIndex().row();
        if (row < 0) {

        } else {
            onItemClicked(row, 0);
        }
    }
}

void PlaybackPicture::initializeChannelTree()
{
    //    if (m_treeType == ChannelTree)
    //    {
    //        return;
    //    }
    //    m_treeType = ChannelTree;

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

    //
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
        ui->treeView->setItemData(row, 0, false, ItemCheckedRole); //m_channelMask>>i&0x01
        ui->treeView->setItemText(row, 1, QString::number(channel + 1));
        ui->treeView->setItemText(row, 2, osdList[channel].name);
        row++;
    }
}

void PlaybackPicture::selectNextChannel()
{
    int channel = ui->treeView->currentIndex().row();
    if (channel < 0) {
        channel = 0;
    } else if (channel < ui->treeView->rowCount() - 1) {
        channel++;
    }
    ui->treeView->setCurrentRow(channel);
}
#if 0

void PlaybackPicture::initializeAlarminTree()
{
    if (m_treeType == AlarmTree)
    {
        return;
    }
    m_treeType = AlarmTree;

    ui->treeView->clearContent();
    //alarmin tree
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CAMERASEARCH/32002", "No.");
    headerList << GET_TEXT("CAMERASTATUS/62002", "Name");
    ui->treeView->setHorizontalHeaderLabels(headerList);
    ui->treeView->setColumnWidth(0, 50);
    ui->treeView->setColumnWidth(1, 100);
    ui->treeView->setHorizontalHeaderCheckable(true);
    ui->treeView->setColumnCount(headerList.count());
    ui->treeView->setItemDelegateForColumn(0, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonCheckBox, this));
    //alarmin
    auto allAlarminNames = qMsNvr->allAlarminNames();
    int row = 0;
    ui->treeView->setRowCount(allAlarminNames.size());
    for (auto iter = allAlarminNames.constBegin(); iter != allAlarminNames.constEnd(); ++iter)
    {
        const AlarmKey &key = iter.key();

        ui->treeView->setItemText(row, 1, key.numberName());
        ui->treeView->setItemData(row, 1, QVariant::fromValue(key), AlarmKeyRole);
        ui->treeView->setItemText(row, 2, key.name());
        row++;
    }
}

void PlaybackPicture::tertiaryTypeInit(int value)
{
    ui->comboBox_tertiaryType->clear();
    ui->label_tertiaryType->show();
    ui->comboBox_tertiaryType->show();
    ui->comboBox_tertiaryType->addItem(GET_TEXT("COMMON/1006","All"), INFO_MINOR_ALL_0);
    if(value == INFO_MAJOR_VCA)
    {
        qDebug()<<"@@@@@@@@@@@ PictureBackup::tertiaryTypeInit(int index):"<<value;
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55001","Region Entrance"), INFO_MINOR_REGIONIN);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55002","Region Exiting"), INFO_MINOR_REGIONOUT);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55003","Advanced Motion Detection"), INFO_MINOR_ADVANCED_MOTION);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55004","Tamper Detection"), INFO_MINOR_TAMPER);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55005","Line Crossing"), INFO_MINOR_LINECROSS);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55006","Loitering"), INFO_MINOR_LOITERING);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/55007","Human Detection"), INFO_MINOR_HUMAN);
        ui->comboBox_tertiaryType->addItem(GET_TEXT("SMARTEVENT/550055","Object Left/Removed"), INFO_MINOR_OBJECT);
    }
    else if(value == INFO_MAJOR_SMART)
    {
        qDebug()<<"@@@@@@@@@@@ PictureBackup::tertiaryTypeInit(int index):"<<value;
        ui->comboBox_tertiaryType->addItem(GET_TEXT("ANPR/103005","ANPR"), 0);
    }
}

void PlaybackPicture::on_comboBox_subType_activated(int index)
{
    int value = ui->comboBox_subType->currentData().toInt();
    qDebug()<<"PlaybackPicture::on_comboBox_subType_activated(int index):"<<index<<";  get:"<<value;
    updateCheckedChannel();
    ui->label_tertiaryType->hide();
    ui->comboBox_tertiaryType->hide();

    switch (value){
    case INFO_MAJOR_ALARMIN:
        qDebug()<<"PlaybackPicture::on_comboBox_subType_activated(int index):INFO_MAJOR_ALARMIN :"<<index<<";  get:"<<value;
        initializeAlarminTree();
        break;
    case INFO_MAJOR_VCA:
        tertiaryTypeInit(value);
        initializeChannelTree();
        break;
    case INFO_MAJOR_SMART:
        tertiaryTypeInit(value);
        initializeChannelTree();
        break;
    default:
        initializeChannelTree();
        break;
    }

    m_subType = value;
}

void PlaybackPicture::updateCheckedChannel()
{
    long long mask = 0;

    if (m_subType == -2)
    {
        m_channelMask = 0;
        m_alarmInputMask = 0;
        return;
    }

    for (int i = 0; i < ui->treeView->rowCount(); ++i)
    {
        const bool &checked = ui->treeView->isItemChecked(i);
        if (checked)
        {
            mask |= (long long)1<<i;
        }
    }

    switch(m_subType) {
    case INFO_MAJOR_MOTION:
    case INFO_MAJOR_VCA:
        m_channelMask = mask;
        break;

    case INFO_MAJOR_ALARMIN:
        m_alarmInputMask = mask;
        break;
    }

    return;
}
#endif
void PlaybackPicture::on_comboBox_mainType_activated(int index)
{
    int value = ui->comboBox_mainType->currentData().toInt();
    qDebug() << "PictureBackup::on_comboBox_pictureType_activated(int index):" << index << ";  get:" << value;
    //    initializeChannelTree();
    if (value == INOF_MAJOR_EVENT_PIC) {
        qDebug() << "########## PlaybackPicture::on_comboBox_mainType_activated(int index):" << index;
        ui->label_subType->show();
        ui->comboBox_subType->show();
        ui->comboBox_subType->setCurrentIndex(0);
    } else {
        ui->label_subType->hide();
        ui->comboBox_subType->hide();
        //        ui->label_tertiaryType->hide();
        //        ui->comboBox_tertiaryType->hide();
    }
}

void PlaybackPicture::onLanguageChanged()
{
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("PLAYBACK/80057", "CH");
    headerList << GET_TEXT("COMMON/1051", "Name");
    ui->treeView->setHorizontalHeaderLabels(headerList);

    //
    ui->label_mainType->setText(GET_TEXT("LOG/64003", "Main Type"));
    ui->comboBox_mainType->retranslate();
    ui->label_subType->setText(GET_TEXT("LOG/64004", "Sub Type"));
    ui->comboBox_subType->retranslate();
    ui->label_startTime->setText(GET_TEXT("LOG/64001", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("LOG/64002", "End Time"));
    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
}

void PlaybackPicture::onItemClicked(const QModelIndex &index)
{
    onItemClicked(index.row(), index.column());
}

void PlaybackPicture::onItemClicked(int row, int column)
{
    Q_UNUSED(column)

    bool checked = ui->treeView->isItemChecked(row);
    checked = !checked;
    ui->treeView->setItemChecked(row, checked);
}

void PlaybackPicture::onTreeViewEnterPressed()
{
    int row = ui->treeView->currentIndex().row();
    if (row < 0) {

    } else {
        onItemClicked(row, 0);
    }
}

void PlaybackPicture::onSearchCanceled()
{
    if (m_searchSid >= 0) {
        sendMessageOnly(REQUEST_FLAG_SEARCH_PIC_BACKUP_CANCEL, &m_searchSid, sizeof(int));
        m_searchSid = -1;
    }
}

void PlaybackPicture::on_pushButton_search_clicked()
{
    //
    const QDateTime startDateTime(ui->dateEdit_start->date(), ui->timeEdit_start->time());
    const QDateTime endDateTime(ui->dateEdit_end->date(), ui->timeEdit_end->time());
    if (startDateTime > endDateTime) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }

    //
    struct req_search_picture_backup search_picture_backup;
    memset(&search_picture_backup, 0, sizeof(struct req_search_picture_backup));

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
    makeChannelMask(checkedList, search_picture_backup.chnMaskl, sizeof(search_picture_backup.chnMaskl));
    search_picture_backup.chnNum = checkedList.size();

#if 0
    const INFO_MINOR_EN &tertiaryType = static_cast<INFO_MINOR_EN>(ui->comboBox_tertiaryType->currentData().toInt());
    switch (m_subType) {
    case INFO_MAJOR_MOTION:
    case INFO_MAJOR_VCA:
    case INFO_MAJOR_SMART:
    {
        QList<int> checkedList;
        for (int i = 0; i < ui->treeView->rowCount(); ++i)
        {
            const bool &checked = ui->treeView->isItemChecked(i);
            if (checked)
            {
                checkedList.append(i);
            }
        }
        if (checkedList.isEmpty())
        {
            ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel"));
            return;
        }
        makeChannelMask(checkedList, search_picture_backup.chnMaskl, sizeof(search_picture_backup.chnMaskl));
        search_picture_backup.chnNum = checkedList.size();

        if (m_subType == INFO_MAJOR_VCA || m_subType == INFO_MAJOR_SMART)
        {
            qDebug()<<QString("on_pushButton_search_clicked:tertiaryType:[%1]").arg(tertiaryType);
        }
    }
    case INFO_MAJOR_ALARMIN:
    {
        QList<AlarmKey> checkedList;
        for (int i = 0; i < ui->treeView->rowCount(); ++i)
        {
            const bool &checked = ui->treeView->itemData(i, 0, ItemCheckedRole).toInt();
            if (checked)
            {
                const AlarmKey &key = ui->treeView->itemData(i, 1, AlarmKeyRole).value<AlarmKey>();
                checkedList.append(key);
            }
        }
        if (checkedList.isEmpty())
        {
            ShowMessageBox(GET_TEXT("EVENTBACKUP/101008","Please select at least one Alarm Input No."));
            return;
        }
//        event_backup.pEventNameLen = MAX_LEN_64;
//        event_backup.enEventCnt = checkedList.count();
        qDebug()<<QString("on_pushButton_search_clicked:INFO_MAJOR_ALARMIN:count:[%1]").arg(checkedList.count());
    }

    default:
        break;
    }
#endif

    search_picture_backup.enMajor = ui->comboBox_mainType->currentData().toInt();
    search_picture_backup.enMinor = ui->comboBox_subType->currentData().toInt();
    if (search_picture_backup.enMajor != INOF_MAJOR_EVENT_PIC)
        search_picture_backup.enMinor = INFO_MINOR_NONE;
    snprintf(search_picture_backup.pStartTime, sizeof(search_picture_backup.pStartTime), "%s", startDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    snprintf(search_picture_backup.pEndTime, sizeof(search_picture_backup.pEndTime), "%s", endDateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    qDebug() << QString("PlaybackPicture enMajor:[%1], enMinor:[%2]").arg(search_picture_backup.enMajor).arg(search_picture_backup.enMinor);
    sendMessage(REQUEST_FLAG_SEARCH_PIC_BACKUP, (void *)&search_picture_backup, sizeof(struct req_search_tags));

    m_isSearching = true;
}
