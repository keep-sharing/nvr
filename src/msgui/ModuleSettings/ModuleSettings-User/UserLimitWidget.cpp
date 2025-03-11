#include "UserLimitWidget.h"
#include "ui_UserLimitWidget.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"

UserLimitWidget::UserLimitWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserLimitWidget)
{
    ui->setupUi(this);

    ui->tabBar->addTab(GET_TEXT("USER/142002", "Operation Permissions"));
    ui->tabBar->addTab(GET_TEXT("USER/142003", "Channel Permissions"));
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabBarClicked(int)));

    initializeLocalMap();
    initializeRemoteMap();

    if (qMsNvr->isSlaveMode()) {
        m_localMap.remove(PERM_RETRIEVE);
        ui->permissionContainerRetrieveLocal->hide();
        m_remoteMap.remove(PERM_RETRIEVE);
        ui->permissionContainerRetrieveRemote->hide();

        m_localMap.remove(PERM_SMART);
        ui->permissionContainerSmartAnalysisLocal->hide();
        m_remoteMap.remove(PERM_SMART);
        ui->permissionContainerSmartAnalysisRemote->hide();

        m_localMap.remove(PERM_CAMERA);
        ui->permissionContainerCameraLocal->hide();
        m_remoteMap.remove(PERM_CAMERA);
        ui->permissionContainerCameraRemote->hide();

        m_localMap.remove(PERM_LIVE_TARGETMODEOPERATION);
        ui->checkBoxTargetModeLocal->hide();
        m_remoteMap.remove(PERM_LIVE_TARGETMODEOPERATION);
        ui->checkBoxTargetModeRemote->hide();

        m_liveViewLocalMap.remove(PERM_LIVE_RECORD);
        ui->checkBoxRecordLocal->hide();
        m_liveViewLocalMap.remove(PERM_LIVE_SNAPSHOT);
        ui->checkBoxLiveViewSnapshotLocal->hide();

        m_playbackLocalMap.remove(PERM_PLAYBACK_SNAPSHOT);
        ui->checkBoxPlaybackSnapshotLocal->hide();
        m_playbackLocalMap.remove(PERM_PLAYBACK_TAG);
        ui->checkBoxTagLocal->hide();
        m_playbackLocalMap.remove(PERM_PLAYBACK_LOCK);
        ui->checkBoxLockLocal->hide();

        m_liveViewRemoteMap.remove(PERM_LIVE_RECORD);
        ui->checkBoxRecordRemote->hide();
        m_liveViewRemoteMap.remove(PERM_LIVE_SNAPSHOT);
        ui->checkBoxLiveViewSnapshotRemote->hide();

        m_playbackRemoteMap.remove(PERM_PLAYBACK_SNAPSHOT);
        ui->checkBoxPlaybackSnapshotRemote->hide();
        m_playbackRemoteMap.remove(PERM_PLAYBACK_TAG);
        ui->checkBoxTagRemote->hide();
        m_playbackRemoteMap.remove(PERM_PLAYBACK_LOCK);
        ui->checkBoxLockRemote->hide();
    }
    if (!qMsNvr->isSupportTalkback()) {
        m_liveViewLocalMap.remove(PERM_LIVE_TWOWAYAUDIO);
        ui->checkBoxTwoWayAudioLocal->hide();
        m_liveViewRemoteMap.remove(PERM_LIVE_TWOWAYAUDIO);
        ui->checkBoxTwoWayAudioRemote->hide();
    }

    if (!qMsNvr->isSupportTargetMode()) {
        m_liveViewLocalMap.remove(PERM_LIVE_TARGETMODEOPERATION);
        ui->checkBoxTargetModeLocal->hide();
        m_liveViewRemoteMap.remove(PERM_LIVE_TARGETMODEOPERATION);
        ui->checkBoxTargetModeRemote->hide();
    }

    ui->checkBoxGroupLiveViewLocal->setCount(qMsNvr->maxChannel(), 4);
    ui->checkBoxGroupLiveViewLocal->setCheckBoxUserStyle();
    ui->checkBoxGroupPlayBackLocal->setCount(qMsNvr->maxChannel(), 4);
    ui->checkBoxGroupPlayBackLocal->setCheckBoxUserStyle();
    ui->checkBoxGroupLiveViewRemote->setCount(qMsNvr->maxChannel(), 4);
    ui->checkBoxGroupLiveViewRemote->setCheckBoxUserStyle();
    ui->checkBoxGroupPlayBackRemote->setCount(qMsNvr->maxChannel(), 4);
    ui->checkBoxGroupPlayBackRemote->setCheckBoxUserStyle();

    //不支持的权限，特殊样式
    ui->checkBoxTargetModeRemote->setProperty("nonsupport", true);
    ui->checkBoxTargetModeRemote->updateStyle();
}

UserLimitWidget::~UserLimitWidget()
{
    delete ui;
}

void UserLimitWidget::initializeUserLimit(const db_user &user)
{
    ui->tabBar->setCurrentTab(0);

    readBasicPermission(user);
    readCameraConfiguration(user);
}

void UserLimitWidget::setReadOnly(bool readOnly)
{
    ui->scrollAreaWidgetContents->setEnabled(!readOnly);
    ui->scrollAreaWidgetContents_2->setEnabled(!readOnly);
}

void UserLimitWidget::onLanguageChanged()
{
    ui->tabBar->setTabText(0, GET_TEXT("USER/142002", "Operation Permissions"));
    ui->tabBar->setTabText(1, GET_TEXT("USER/142003", "Channel Permissions"));
    ui->labelLocal->setText(GET_TEXT("USER/74054", "Local"));
    ui->labelRemote->setText(GET_TEXT("USER/142008", "Remote"));
    ui->labelChannelLocal->setText(GET_TEXT("USER/74054", "Local"));
    ui->labelChannelRemote->setText(GET_TEXT("USER/142008", "Remote"));

    ui->checkBoxGroupLiveViewLocal->setAllButtonText(GET_TEXT("CHANNELMANAGE/30006", "Live View"));
    ui->checkBoxGroupPlayBackLocal->setAllButtonText(GET_TEXT("MENU/10002", "Playback"));
    ui->checkBoxGroupLiveViewRemote->setAllButtonText(GET_TEXT("CHANNELMANAGE/30006", "Live View"));
    ui->checkBoxGroupPlayBackRemote->setAllButtonText(GET_TEXT("MENU/10002", "Playback"));

    ui->checkBoxAllLocal->setText(GET_TEXT("SYSTEMGENERAL/164000", "All"));
    ui->checkBoxAllRemote->setText(GET_TEXT("SYSTEMGENERAL/164000", "All"));

    ui->checkBoxLiveViewLocal->setText(GET_TEXT("USER/142004", "Live View Operation"));
    ui->checkBoxLiveViewRemote->setText(GET_TEXT("USER/142004", "Live View Operation"));
    ui->checkBoxRecordLocal->setText(GET_TEXT("MENU/10004", "Record"));
    ui->checkBoxRecordRemote->setText(GET_TEXT("MENU/10004", "Record"));
    ui->checkBoxLiveViewSnapshotLocal->setText(GET_TEXT("LIVEVIEW/20025", "Snapshot"));
    ui->checkBoxLiveViewSnapshotRemote->setText(GET_TEXT("LIVEVIEW/20025", "Snapshot"));
    ui->checkBoxLiveViewAudioLocal->setText(GET_TEXT("USER/74042", "Audio"));
    ui->checkBoxLiveViewAudioRemote->setText(GET_TEXT("USER/74042", "Audio"));
    ui->checkBoxTwoWayAudioLocal->setText(GET_TEXT("LIVEVIEW/20104", "Two-way Audio"));
    ui->checkBoxTwoWayAudioRemote->setText(GET_TEXT("LIVEVIEW/20104", "Two-way Audio"));
    ui->checkBoxPTZControlLocal->setText(GET_TEXT("USER/74028", "PTZ Control"));
    ui->checkBoxPTZControlRemote->setText(GET_TEXT("USER/74028", "PTZ Control"));
    ui->checkBoxPTZSettingLocal->setText(GET_TEXT("USER/74030", "PTZ Settings"));
    ui->checkBoxPTZSettingRemote->setText(GET_TEXT("USER/74030", "PTZ Settings"));
    ui->checkBoxImageConfigurationLocal->setText(GET_TEXT("LIVEVIEW/20020", "Image Configuration"));
    ui->checkBoxImageConfigurationRemote->setText(GET_TEXT("LIVEVIEW/20020", "Image Configuration"));
    ui->checkBoxAlarmOutputLocal->setText(GET_TEXT("ALARMOUT/53013", "Camera Alarm Output"));
    ui->checkBoxAlarmOutputRemote->setText(GET_TEXT("ALARMOUT/53013", "Camera Alarm Output"));
    ui->checkBoxPlayModeLocal->setText(GET_TEXT("LIVEPARAMETER/41010", "Play Mode"));
    ui->checkBoxPlayModeRemote->setText(GET_TEXT("LIVEPARAMETER/41010", "Play Mode"));
    ui->checkBoxTargetModeLocal->setText(GET_TEXT("USER/142009", "Target Mode Operation"));
    ui->checkBoxTargetModeRemote->setText(GET_TEXT("USER/142009", "Target Mode Operation"));

    ui->checkBoxPlaybackLocal->setText(GET_TEXT("USER/142005", "Playback Operation"));
    ui->checkBoxPlaybackRemote->setText(GET_TEXT("USER/142005", "Playback Operation"));
    ui->checkBoxPlaybackSnapshotLocal->setText(GET_TEXT("LIVEVIEW/20025", "Snapshot"));
    ui->checkBoxPlaybackSnapshotRemote->setText(GET_TEXT("LIVEVIEW/20025", "Snapshot"));
    ui->checkBoxPlaybackAudioLocal->setText(GET_TEXT("USER/74042", "Audio"));
    ui->checkBoxPlaybackAudioRemote->setText(GET_TEXT("USER/74042", "Audio"));
    ui->checkBoxLockLocal->setText(GET_TEXT("LIVEVIEW/20060", "Lock"));
    ui->checkBoxLockRemote->setText(GET_TEXT("LIVEVIEW/20060", "Lock"));
    ui->checkBoxFileExportLocal->setText(GET_TEXT("USER/142011", "File Export"));
    ui->checkBoxFileExportRemote->setText(GET_TEXT("USER/142011","File Export"));
    ui->checkBoxTagLocal->setText(GET_TEXT("PLAYBACK/80076","Tag"));
    ui->checkBoxTagRemote->setText(GET_TEXT("PLAYBACK/80076","Tag"));

    ui->checkBoxRetrieveLocal->setText(GET_TEXT("MENU/10014", "Retrieve"));
    ui->checkBoxRetrieveRemote->setText(GET_TEXT("MENU/10014", "Retrieve"));
    ui->checkBoxSmartAnalysisLocal->setText(GET_TEXT("ANPR/103054", "Smart Analysis"));
    ui->checkBoxSmartAnalysisRemote->setText(GET_TEXT("ANPR/103054", "Smart Analysis"));
    ui->checkBoxCameraLocal->setText(GET_TEXT("MENU/10003", "Camera"));
    ui->checkBoxCameraRemote->setText(GET_TEXT("MENU/10003", "Camera"));
    ui->checkBoxStorageLocal->setText(GET_TEXT("RECORDADVANCE/91019", "Storage"));
    ui->checkBoxStorageRemote->setText(GET_TEXT("RECORDADVANCE/91019", "Storage"));
    ui->checkBoxEventLocal->setText(GET_TEXT("MENU/10005", "Event"));
    ui->checkBoxEventRemote->setText(GET_TEXT("MENU/10005", "Event"));
    ui->checkBoxSettingLocal->setText(GET_TEXT("MENU/10007", "Settings"));
    ui->checkBoxSettingRemote->setText(GET_TEXT("USER/142010", "System"));
    ui->checkBoxStatusLocal->setText(GET_TEXT("USER/74033", "Status & Log"));
    ui->checkBoxStatusRemote->setText(GET_TEXT("USER/74033", "Status & Log"));
    ui->checkBoxShutdownLocal->setText(GET_TEXT("USER/74040", "Shutdown/Reboot"));
    ui->checkBoxShutdownRemote->setText(GET_TEXT("USER/74045", "Reboot"));
}

void UserLimitWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void UserLimitWidget::initializeLocalMap()
{
    m_liveViewLocalMap.insert(PERM_LIVE_RECORD, ui->checkBoxRecordLocal);
    m_liveViewLocalMap.insert(PERM_LIVE_SNAPSHOT, ui->checkBoxLiveViewSnapshotLocal);
    m_liveViewLocalMap.insert(PERM_LIVE_AUDIO, ui->checkBoxLiveViewAudioLocal);
    m_liveViewLocalMap.insert(PERM_LIVE_TWOWAYAUDIO, ui->checkBoxTwoWayAudioLocal);
    m_liveViewLocalMap.insert(PERM_LIVE_PTZCONTROL, ui->checkBoxPTZControlLocal);
    m_liveViewLocalMap.insert(PERM_LIVE_PTZSETTINGS, ui->checkBoxPTZSettingLocal);
    m_liveViewLocalMap.insert(PERM_LIVE_IMAGE, ui->checkBoxImageConfigurationLocal);
    m_liveViewLocalMap.insert(PERM_LIVE_CAMERAALARMOUTPUT, ui->checkBoxAlarmOutputLocal);
    m_liveViewLocalMap.insert(PERM_LIVE_PLAYMODE, ui->checkBoxPlayModeLocal);
    m_liveViewLocalMap.insert(PERM_LIVE_TARGETMODEOPERATION, ui->checkBoxTargetModeLocal);
    for (auto iter = m_liveViewLocalMap.constBegin(); iter != m_liveViewLocalMap.constEnd(); ++iter) {
        QCheckBox *box = iter.value();
        box->setMinimumHeight(25);
    }

    m_playbackLocalMap.insert(PERM_PLAYBACK_SNAPSHOT, ui->checkBoxPlaybackSnapshotLocal);
    m_playbackLocalMap.insert(PERM_PLAYBACK_AUDIO, ui->checkBoxPlaybackAudioLocal);
    m_playbackLocalMap.insert(PERM_PLAYBACK_TAG, ui->checkBoxTagLocal);
    m_playbackLocalMap.insert(PERM_PLAYBACK_LOCK, ui->checkBoxLockLocal);
    m_playbackLocalMap.insert(PERM_PLAYBACK_FILEEXPORT, ui->checkBoxFileExportLocal);

    m_localMap.insert(PERM_LIVE, ui->checkBoxLiveViewLocal);
    m_localMap.insert(PERM_PLAYBACK, ui->checkBoxPlaybackLocal);
    m_localMap.insert(PERM_RETRIEVE, ui->checkBoxRetrieveLocal);
    m_localMap.insert(PERM_SMART, ui->checkBoxSmartAnalysisLocal);
    m_localMap.insert(PERM_CAMERA, ui->checkBoxCameraLocal);
    m_localMap.insert(PERM_STORAGE, ui->checkBoxStorageLocal);
    m_localMap.insert(PERM_EVENT, ui->checkBoxEventLocal);
    m_localMap.insert(PERM_SETTINGS, ui->checkBoxSettingLocal);
    m_localMap.insert(PERM_STATUS, ui->checkBoxStatusLocal);
    m_localMap.insert(PERM_SHUTDOWN, ui->checkBoxShutdownLocal);
}

void UserLimitWidget::initializeRemoteMap()
{
    m_liveViewRemoteMap.insert(PERM_LIVE_RECORD, ui->checkBoxRecordRemote);
    m_liveViewRemoteMap.insert(PERM_LIVE_SNAPSHOT, ui->checkBoxLiveViewSnapshotRemote);
    m_liveViewRemoteMap.insert(PERM_LIVE_AUDIO, ui->checkBoxLiveViewAudioRemote);
    m_liveViewRemoteMap.insert(PERM_LIVE_TWOWAYAUDIO, ui->checkBoxTwoWayAudioRemote);
    m_liveViewRemoteMap.insert(PERM_LIVE_PTZCONTROL, ui->checkBoxPTZControlRemote);
    m_liveViewRemoteMap.insert(PERM_LIVE_PTZSETTINGS, ui->checkBoxPTZSettingRemote);
    m_liveViewRemoteMap.insert(PERM_LIVE_IMAGE, ui->checkBoxImageConfigurationRemote);
    m_liveViewRemoteMap.insert(PERM_LIVE_CAMERAALARMOUTPUT, ui->checkBoxAlarmOutputRemote);
    m_liveViewRemoteMap.insert(PERM_LIVE_PLAYMODE, ui->checkBoxPlayModeRemote);
    ui->checkBoxTargetModeRemote->setEnabled(false);

    m_playbackRemoteMap.insert(PERM_PLAYBACK_SNAPSHOT, ui->checkBoxPlaybackSnapshotRemote);
    m_playbackRemoteMap.insert(PERM_PLAYBACK_AUDIO, ui->checkBoxPlaybackAudioRemote);
    m_playbackRemoteMap.insert(PERM_PLAYBACK_TAG, ui->checkBoxTagRemote);
    m_playbackRemoteMap.insert(PERM_PLAYBACK_LOCK, ui->checkBoxLockRemote);
    m_playbackRemoteMap.insert(PERM_PLAYBACK_FILEEXPORT, ui->checkBoxFileExportRemote);

    m_remoteMap.insert(PERM_LIVE, ui->checkBoxLiveViewRemote);
    m_remoteMap.insert(PERM_PLAYBACK, ui->checkBoxPlaybackRemote);
    m_remoteMap.insert(PERM_RETRIEVE, ui->checkBoxRetrieveRemote);
    m_remoteMap.insert(PERM_SMART, ui->checkBoxSmartAnalysisRemote);
    m_remoteMap.insert(PERM_CAMERA, ui->checkBoxCameraRemote);
    m_remoteMap.insert(PERM_STORAGE, ui->checkBoxStorageRemote);
    m_remoteMap.insert(PERM_EVENT, ui->checkBoxEventRemote);
    m_remoteMap.insert(PERM_SETTINGS, ui->checkBoxSettingRemote);
    m_remoteMap.insert(PERM_STATUS, ui->checkBoxStatusRemote);
    m_remoteMap.insert(PERM_SHUTDOWN, ui->checkBoxShutdownRemote);
}

void UserLimitWidget::readBasicPermission(const db_user &user)
{
    //local
    if (user.perm_local_live == PERM_LIVE_ALL) {
        on_checkBoxLiveViewLocal_clicked(true);
    } else if (user.perm_local_live == PERM_LIVE_NONE) {
        on_checkBoxLiveViewLocal_clicked(false);
    } else {
        for (auto iter = m_liveViewLocalMap.constBegin(); iter != m_liveViewLocalMap.constEnd(); ++iter) {
            int permission = iter.key();
            QCheckBox *checkBox = iter.value();
            if (user.perm_local_live & permission) {
                checkBox->setChecked(true);
            } else {
                checkBox->setChecked(false);
            }
        }
    }
    onLiveViewLocalCheckBoxClicked(true);

    if (user.perm_local_playback == PERM_PLAYBACK_ALL) {
        on_checkBoxPlaybackLocal_clicked(true);
    } else if (user.perm_local_playback == PERM_PLAYBACK_NONE) {
        on_checkBoxPlaybackLocal_clicked(false);
    } else {
        for (auto iter = m_playbackLocalMap.constBegin(); iter != m_playbackLocalMap.constEnd(); ++iter) {
            int permission = iter.key();
            QCheckBox *checkBox = iter.value();
            if (user.perm_local_playback & permission) {
                checkBox->setChecked(true);
            } else {
                checkBox->setChecked(false);
            }
        }
    }
    onPlaybackLocalCheckBoxClicked(true);

    ui->checkBoxRetrieveLocal->setChecked(user.perm_local_retrieve == PERM_RETRIEVE_ALL);
    ui->checkBoxSmartAnalysisLocal->setChecked(user.perm_local_smart == PERM_SMART_ALL);
    ui->checkBoxCameraLocal->setChecked(user.perm_local_camera == PERM_CAMERA_ALL);
    ui->checkBoxStorageLocal->setChecked(user.perm_local_storage == PERM_STORAGE_ALL);
    ui->checkBoxEventLocal->setChecked(user.perm_local_event == PERM_EVENT_ALL);
    ui->checkBoxSettingLocal->setChecked(user.perm_local_settings == PERM_SETTINGS_ALL);
    ui->checkBoxStatusLocal->setChecked(user.perm_local_status == PERM_STATUS_ALL);
    ui->checkBoxShutdownLocal->setChecked(user.perm_local_shutdown == PERM_SHUTDOWN_ALL);
    onLocalCheckBoxClicked(true);

    //remote
    if (user.perm_remote_live == PERM_LIVE_ALL) {
        on_checkBoxLiveViewRemote_clicked(true);
    } else if (user.perm_remote_live == PERM_LIVE_NONE) {
        on_checkBoxLiveViewRemote_clicked(false);
    } else {
        for (auto iter = m_liveViewRemoteMap.constBegin(); iter != m_liveViewRemoteMap.constEnd(); ++iter) {
            int permission = iter.key();
            QCheckBox *checkBox = iter.value();
            if (user.perm_remote_live & permission) {
                checkBox->setChecked(true);
            } else {
                checkBox->setChecked(false);
            }
        }
    }
    onLiveViewRemoteCheckBoxClicked(true);

    if (user.perm_remote_playback == PERM_PLAYBACK_ALL) {
        on_checkBoxPlaybackRemote_clicked(true);
    } else if (user.perm_remote_playback == PERM_PLAYBACK_NONE) {
        on_checkBoxPlaybackRemote_clicked(false);
    } else {
        for (auto iter = m_playbackRemoteMap.constBegin(); iter != m_playbackRemoteMap.constEnd(); ++iter) {
            int permission = iter.key();
            QCheckBox *checkBox = iter.value();
            if (user.perm_remote_playback & permission) {
                checkBox->setChecked(true);
            } else {
                checkBox->setChecked(false);
            }
        }
    }
    onPlaybackRemoteCheckBoxClicked(true);

    ui->checkBoxRetrieveRemote->setChecked(user.perm_remote_retrieve == PERM_RETRIEVE_ALL);
    ui->checkBoxSmartAnalysisRemote->setChecked(user.perm_remote_smart == PERM_SMART_ALL);
    ui->checkBoxCameraRemote->setChecked(user.perm_remote_camera == PERM_CAMERA_ALL);
    ui->checkBoxStorageRemote->setChecked(user.perm_remote_storage == PERM_STORAGE_ALL);
    ui->checkBoxEventRemote->setChecked(user.perm_remote_event == PERM_EVENT_ALL);
    ui->checkBoxSettingRemote->setChecked(user.perm_remote_settings == PERM_SETTINGS_ALL);
    ui->checkBoxStatusRemote->setChecked(user.perm_remote_status == PERM_STATUS_ALL);
    ui->checkBoxShutdownRemote->setChecked(user.perm_remote_shutdown == PERM_SHUTDOWN_ALL);
    onRemoteCheckBoxClicked(true);
}

void UserLimitWidget::readCameraConfiguration(const db_user &user)
{
    //local liveview
    ui->checkBoxGroupLiveViewLocal->setCheckedFromString(QString(user.local_live_view_ex));

    //local playback
    ui->checkBoxGroupPlayBackLocal->setCheckedFromString(QString(user.local_playback_ex));

    //remote liveview
    ui->checkBoxGroupLiveViewRemote->setCheckedFromString(QString(user.remote_live_view_ex));

    //remote playback
    ui->checkBoxGroupPlayBackRemote->setCheckedFromString(QString(user.remote_playback_ex));
}

void UserLimitWidget::onTabBarClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void UserLimitWidget::onLocalCheckBoxClicked(bool checked)
{
    Q_UNUSED(checked)
    int localCheckedCount = 0;
    for (auto iter = m_localMap.constBegin(); iter != m_localMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();
        if (checkBox->checkState() == Qt::Checked) {
            localCheckedCount++;
        }
    }
    if (localCheckedCount == 0) {
        ui->checkBoxAllLocal->setCheckState(Qt::Unchecked);
    } else if (localCheckedCount == m_localMap.size()) {
        ui->checkBoxAllLocal->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxAllLocal->setCheckState(Qt::PartiallyChecked);
    }
}

void UserLimitWidget::onRemoteCheckBoxClicked(bool checked)
{
    Q_UNUSED(checked)
    int remoteCheckedCount = 0;
    for (auto iter = m_remoteMap.constBegin(); iter != m_remoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checkBox->checkState() == Qt::Checked) {
            remoteCheckedCount++;
        }
    }
    if (remoteCheckedCount == 0) {
        ui->checkBoxAllRemote->setCheckState(Qt::Unchecked);
    } else if (remoteCheckedCount == m_remoteMap.size()) {
        ui->checkBoxAllRemote->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxAllRemote->setCheckState(Qt::PartiallyChecked);
    }
}

void UserLimitWidget::onLiveViewLocalCheckBoxClicked(bool checked)
{
    Q_UNUSED(checked)
    int localCheckedCount = 0;
    for (auto iter = m_liveViewLocalMap.constBegin(); iter != m_liveViewLocalMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checkBox->isChecked()) {
            localCheckedCount++;
        }
    }
    if (localCheckedCount == 0) {
        ui->checkBoxLiveViewLocal->setCheckState(Qt::Unchecked);
    } else if (localCheckedCount == m_liveViewLocalMap.count()) {
        ui->checkBoxLiveViewLocal->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxLiveViewLocal->setCheckState(Qt::PartiallyChecked);
    }
    onLocalCheckBoxClicked(true);
}

void UserLimitWidget::onLiveViewRemoteCheckBoxClicked(bool checked)
{
    Q_UNUSED(checked)
    int remoteCheckedCount = 0;
    for (auto iter = m_liveViewRemoteMap.constBegin(); iter != m_liveViewRemoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();
        if (checkBox->isChecked()) {
            remoteCheckedCount++;
        }
    }
    if (remoteCheckedCount == 0) {
        ui->checkBoxLiveViewRemote->setCheckState(Qt::Unchecked);
    } else if (remoteCheckedCount == m_liveViewRemoteMap.count()) {
        ui->checkBoxLiveViewRemote->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxLiveViewRemote->setCheckState(Qt::PartiallyChecked);
    }
    onRemoteCheckBoxClicked(true);
}

void UserLimitWidget::onPlaybackLocalCheckBoxClicked(bool checked)
{
    Q_UNUSED(checked)
    int localCheckedCount = 0;
    for (auto iter = m_playbackLocalMap.constBegin(); iter != m_playbackLocalMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checkBox->isChecked()) {
            localCheckedCount++;
        }
    }
    if (localCheckedCount == 0) {
        ui->checkBoxPlaybackLocal->setCheckState(Qt::Unchecked);
    } else if (localCheckedCount == m_playbackLocalMap.count()) {
        ui->checkBoxPlaybackLocal->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxPlaybackLocal->setCheckState(Qt::PartiallyChecked);
    }
    onLocalCheckBoxClicked(true);
}

void UserLimitWidget::onPlaybackRemoteCheckBoxClicked(bool checked)
{
    Q_UNUSED(checked)
    int remoteCheckedCount = 0;
    for (auto iter = m_playbackRemoteMap.constBegin(); iter != m_playbackRemoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checkBox->isChecked()) {
            remoteCheckedCount++;
        }
    }
    if (remoteCheckedCount == 0) {
        ui->checkBoxPlaybackRemote->setCheckState(Qt::Unchecked);
    } else if (remoteCheckedCount == m_playbackRemoteMap.count()) {
        ui->checkBoxPlaybackRemote->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxPlaybackRemote->setCheckState(Qt::PartiallyChecked);
    }
    onRemoteCheckBoxClicked(true);
}

void UserLimitWidget::on_checkBoxLiveViewLocal_clicked(bool checked)
{
    for (auto iter = m_liveViewLocalMap.constBegin(); iter != m_liveViewLocalMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checked) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }
    }
    if (checked) {
        ui->checkBoxLiveViewLocal->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxLiveViewLocal->setCheckState(Qt::Unchecked);
    }
}

void UserLimitWidget::on_checkBoxLiveViewRemote_clicked(bool checked)
{
    for (auto iter = m_liveViewRemoteMap.constBegin(); iter != m_liveViewRemoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checked) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }
    }
    if (checked) {
        ui->checkBoxLiveViewRemote->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxLiveViewRemote->setCheckState(Qt::Unchecked);
    }
}

void UserLimitWidget::on_checkBoxPlaybackLocal_clicked(bool checked)
{
    for (auto iter = m_playbackLocalMap.constBegin(); iter != m_playbackLocalMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checked) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }
    }
    if (checked) {
        ui->checkBoxPlaybackLocal->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxPlaybackLocal->setCheckState(Qt::Unchecked);
    }
}

void UserLimitWidget::on_checkBoxPlaybackRemote_clicked(bool checked)
{
    for (auto iter = m_playbackRemoteMap.constBegin(); iter != m_playbackRemoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checked) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }
    }
    if (checked) {
        ui->checkBoxPlaybackRemote->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxPlaybackRemote->setCheckState(Qt::Unchecked);
    }
}
