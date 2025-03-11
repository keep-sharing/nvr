#include "PlaybackFileManagement.h"
#include "ui_PlaybackFileManagement.h"
#include "BasePlayback.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "msuser.h"
#include "MyFileSystemDialog.h"
#include <QFile>

PlaybackFileManagement *PlaybackFileManagement::s_fileManagement = nullptr;

PlaybackFileManagement::PlaybackFileManagement(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::PlaybackFileManagement)
{
    ui->setupUi(this);

    s_fileManagement = this;

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    } else {
        qWarning() << QString("PlaybackFileManagement load style failed.");
    }
    file.close();

    connect(ui->pushButton_cancel, SIGNAL(clicked(bool)), this, SLOT(onPushButtonCancelClicked()));

    //
    ui->tabBar->addTab(GET_TEXT("PLAYBACK/80074", "Video Clip"));
    if (!qMsNvr->isSlaveMode()) {
        ui->tabBar->addTab(GET_TEXT("PICTUREBACKUP/102001", "Playback Snapshot"));
        ui->tabBar->addTab(GET_TEXT("PLAYBACK/80075", "Locked File"));
        ui->tabBar->addTab(GET_TEXT("PLAYBACK/80076", "Tag"));
    }
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
    ui->tabBar->setCurrentTab(0);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PlaybackFileManagement::~PlaybackFileManagement()
{
    s_fileManagement = nullptr;
    delete ui;
}

PlaybackFileManagement *PlaybackFileManagement::instance()
{
    return s_fileManagement;
}

void PlaybackFileManagement::close()
{
    setCurrentMode(ModeNone);
    BaseShadowDialog::close();
}

bool PlaybackFileManagement::hasFreeSid()
{
    return AbstractPlaybackFile::hasFreeSid();
}

void PlaybackFileManagement::cleanup()
{
    ui->page_video->clear();
    ui->page_snapshot->clear();
    ui->page_lock->clear();
    ui->page_tag->clear();
    //
    AbstractPlaybackFile::closeAllSid();
}

bool PlaybackFileManagement::hasNotExportedVideoFile() const
{
    return ui->page_video->hasNotExportedFile();
}

bool PlaybackFileManagement::hasNotExportedPictureFile() const
{
    return ui->page_snapshot->hasNotExportedFile();
}

/**
 * @brief PlaybackFileManagement::waitForSnapshot
 * @return 成功的通道个数
 */
int PlaybackFileManagement::waitForSnapshot(int winid)
{
    return ui->page_snapshot->waitForSnapshot(winid);
}

/**
 * @brief PlaybackFileManagement::waitForCutPlayback
 * @param begin
 * @param end
 * @return MsPlaybackError
 */
int PlaybackFileManagement::waitForCutPlayback(const QDateTime &begin, const QDateTime &end)
{
    return ui->page_video->waitForCutPlayback(begin, end);
}

int PlaybackFileManagement::addLockFile(const resp_search_common_backup &common_backup)
{
    return ui->page_lock->addLockFile(common_backup);
}

/**
 * @brief PlaybackFileManagement::waitForLockPlayback
 * @param dateTime
 * @return 成功的通道个数
 */
int PlaybackFileManagement::waitForPlaybackLockAll(const QDateTime &dateTime)
{
    return ui->page_lock->waitForLockAll(dateTime);
}

int PlaybackFileManagement::waitForPlaybackLock(const QDateTime &dateTime)
{
    return ui->page_lock->waitForLock(dateTime);
}

/**
 * @brief PlaybackFileManagement::waitForTag
 * @param dateTime
 * @param name
 * @return 成功的通道个数
 */
int PlaybackFileManagement::waitForTagAll(const QMap<int, QDateTime> &dateTimeMap, const QString &name)
{
    return ui->page_tag->waitForTagAll(dateTimeMap, name);
}

int PlaybackFileManagement::waitForTag(const QDateTime &dateTime, const QString &name)
{
    return ui->page_tag->waitForTag(dateTime, name);
}

void PlaybackFileManagement::initializeData()
{
    onTabClicked(ui->tabBar->currentTab());
}

void PlaybackFileManagement::setCurrentMode(PlaybackFileManagement::Mode mode)
{
    m_mode = mode;

    switch (m_mode) {
    case ModeVideo:
        ui->tabBar->setCurrentTab(0);
        break;
    case ModeSnapshot:
        ui->tabBar->setCurrentTab(1);
        break;
    case ModeLock:
        ui->tabBar->setCurrentTab(2);
        break;
    case ModeTag:
        ui->tabBar->setCurrentTab(3);
        break;
    default:
        break;
    }
}

void PlaybackFileManagement::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    switch (m_mode) {
    case ModeVideo:
        ui->page_video->dealMessage(message);
        break;
    case ModeSnapshot:
        ui->page_snapshot->dealMessage(message);
        break;
    case ModeLock:
        ui->page_lock->dealMessage(message);
        break;
    case ModeTag:
        ui->page_tag->dealMessage(message);
        break;
    default:
        break;
    }
}

void PlaybackFileManagement::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("PLAYBACK/80104", "Playback File Management"));

    ui->tabBar->setTabText(0, GET_TEXT("PLAYBACK/80074", "Video Clip"));
    ui->tabBar->setTabText(1, GET_TEXT("PICTUREBACKUP/102001", "Playback Snapshot"));
    ui->tabBar->setTabText(2, GET_TEXT("PLAYBACK/80075", "Locked File"));
    ui->tabBar->setTabText(3, GET_TEXT("PLAYBACK/80076", "Tag"));

    ui->pushButton_exportAll->setText(GET_TEXT("PLAYBACK/80073", "Export All"));
    ui->pushButton_export->setText(GET_TEXT("LOG/64013", "Export"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    ui->pushButton_deleteAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));
    ui->pushButton_delete->setText(GET_TEXT("CHANNELMANAGE/30023", "Delete"));
}

void PlaybackFileManagement::onTabClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);

    switch (index) {
    case 0:
        m_mode = ModeVideo;
        ui->page_video->updateTableList();
        break;
    case 1:
        m_mode = ModeSnapshot;
        ui->page_snapshot->updateTableList();
        break;
    case 2:
        m_mode = ModeLock;
        ui->page_lock->updateTableList();
        break;
    case 3:
        m_mode = ModeTag;
        break;
    }
    if (index == 3) {
        ui->pushButton_export->hide();
        ui->pushButton_exportAll->hide();
        ui->pushButton_deleteAll->show();
        ui->pushButton_delete->show();
    } else {
        ui->pushButton_export->show();
        ui->pushButton_exportAll->show();
        ui->pushButton_deleteAll->hide();
        ui->pushButton_delete->hide();
    }
}

void PlaybackFileManagement::on_pushButton_exportAll_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_FILEEXPORT)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    switch (m_mode) {
    case ModeVideo:
        ui->page_video->exportAllVideo();
        break;
    case ModeSnapshot:
        ui->page_snapshot->exportAllSnapshot();
        break;
    case ModeLock:
        ui->page_lock->exportAllLock();
        break;
    default:
        break;
    }
}

void PlaybackFileManagement::on_pushButton_export_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_PLAYBACK, PERM_PLAYBACK_FILEEXPORT)) {
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    switch (m_mode) {
    case ModeVideo:
        ui->page_video->exportVideo();
        break;
    case ModeSnapshot:
        ui->page_snapshot->exportSnapshot();
        break;
    case ModeLock:
        ui->page_lock->exportLock();
        break;
    default:
        break;
    }
}

void PlaybackFileManagement::onPushButtonCancelClicked()
{
    close();
}

void PlaybackFileManagement::on_pushButton_deleteAll_clicked()
{
    switch (m_mode) {
    case ModeTag:
        ui->page_tag->deleteAllTag();
        break;
    default:
        break;
    }
}

void PlaybackFileManagement::on_pushButton_delete_clicked()
{
    switch (m_mode) {
    case ModeTag:
        ui->page_tag->deleteTag();
        break;
    default:
        break;
    }
}
