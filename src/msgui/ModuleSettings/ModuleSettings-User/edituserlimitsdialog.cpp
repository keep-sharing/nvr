#include "edituserlimitsdialog.h"
#include "ui_edituserlimitsdialog.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include <QCryptographicHash>

EditUserLimitsDialog::EditUserLimitsDialog(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::EditUserLimitsDialog)
{
    ui->setupUi(this);

    ui->tabBar->addTab(GET_TEXT("USER/142002", "Operation Permissions"));
    ui->tabBar->addTab(GET_TEXT("USER/142003", "Channel Permissions"));
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabBarClicked(int)));

    initializeLocalMap();
    initializeRemoteMap();

    if (qMsNvr->isSlaveMode()) {
        m_localMap.remove(PERM_RETRIEVE);
        ui->checkBoxRetrieveLocal->hide();
        m_remoteMap.remove(PERM_RETRIEVE);
        ui->checkBoxRetrieveRemote->hide();

        m_localMap.remove(PERM_SMART);
        ui->checkBoxSmartAnalysisLocal->hide();
        m_remoteMap.remove(PERM_SMART);
        ui->checkBoxSmartAnalysisRemote->hide();

        m_localMap.remove(PERM_CAMERA);
        ui->checkBoxCameraLocal->hide();
        m_remoteMap.remove(PERM_CAMERA);
        ui->checkBoxCameraRemote->hide();

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
    ui->checkBoxGroupLiveViewLocal->setCheckBoxStyle();
    ui->checkBoxGroupPlayBackLocal->setCount(qMsNvr->maxChannel(), 4);
    ui->checkBoxGroupPlayBackLocal->setCheckBoxStyle();
    ui->checkBoxGroupLiveViewRemote->setCount(qMsNvr->maxChannel(), 4);
    ui->checkBoxGroupLiveViewRemote->setCheckBoxStyle();
    ui->checkBoxGroupPlayBackRemote->setCount(qMsNvr->maxChannel(), 4);
    ui->checkBoxGroupPlayBackRemote->setCheckBoxStyle();

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

EditUserLimitsDialog::~EditUserLimitsDialog()
{
    delete ui;
}

void EditUserLimitsDialog::setEditUserLimitInit(const db_user &user)
{
    memcpy(&m_user, &user, sizeof(db_user));

    readFromProfileToDlg();
    readFromProfileToDlgCameraConfiguration();

    ui->tabBar->setCurrentTab(0);
}

void EditUserLimitsDialog::getEditUserLimitInit(db_user &user)
{
    memcpy(&user, &m_user, sizeof(db_user));
}

void EditUserLimitsDialog::initializeLocalMap()
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

    for (auto iter = m_localMap.constBegin(); iter != m_localMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onLocalCheckBoxClicked(bool)));
    }

    for (auto iter = m_liveViewLocalMap.constBegin(); iter != m_liveViewLocalMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onLiveViewLocalCheckBoxClicked(bool)));
    }

    for (auto iter = m_playbackLocalMap.constBegin(); iter != m_playbackLocalMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onPlaybackLocalCheckBoxClicked(bool)));
    }
}

void EditUserLimitsDialog::initializeRemoteMap()
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

    for (auto iter = m_remoteMap.constBegin(); iter != m_remoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onRemoteCheckBoxClicked(bool)));
    }

    for (auto iter = m_liveViewRemoteMap.constBegin(); iter != m_liveViewRemoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onLiveViewRemoteCheckBoxClicked(bool)));
    }

    for (auto iter = m_playbackRemoteMap.constBegin(); iter != m_playbackRemoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onPlaybackRemoteCheckBoxClicked(bool)));
    }
}

void EditUserLimitsDialog::readFromProfileToDlg()
{
    //local
    if (m_user.perm_local_live == PERM_LIVE_ALL) {
        on_checkBoxLiveViewLocal_clicked(true);
    } else if (m_user.perm_local_live == PERM_LIVE_NONE) {
        on_checkBoxLiveViewLocal_clicked(false);
    } else {
        for (auto iter = m_liveViewLocalMap.constBegin(); iter != m_liveViewLocalMap.constEnd(); ++iter) {
            int permission = iter.key();
            QCheckBox *checkBox = iter.value();
            if (m_user.perm_local_live & permission && checkBox->isEnabled()) {
                checkBox->setChecked(true);
            } else {
                checkBox->setChecked(false);
            }
        }
    }
    onLiveViewLocalCheckBoxClicked(true);

    if (m_user.perm_local_playback == PERM_PLAYBACK_ALL) {
        on_checkBoxPlaybackLocal_clicked(true);
    } else if (m_user.perm_local_playback == PERM_PLAYBACK_NONE) {
        on_checkBoxPlaybackLocal_clicked(false);
    } else {
        for (auto iter = m_playbackLocalMap.constBegin(); iter != m_playbackLocalMap.constEnd(); ++iter) {
            int permission = iter.key();
            QCheckBox *checkBox = iter.value();
            if (m_user.perm_local_playback & permission && checkBox->isEnabled()) {
                checkBox->setChecked(true);
            } else {
                checkBox->setChecked(false);
            }
        }
    }
    onPlaybackLocalCheckBoxClicked(true);

    ui->checkBoxRetrieveLocal->setChecked(m_user.perm_local_retrieve == PERM_RETRIEVE_ALL && ui->checkBoxRetrieveLocal->isEnabled());
    ui->checkBoxSmartAnalysisLocal->setChecked(m_user.perm_local_smart == PERM_SMART_ALL && ui->checkBoxSmartAnalysisLocal->isEnabled());
    ui->checkBoxCameraLocal->setChecked(m_user.perm_local_camera == PERM_CAMERA_ALL && ui->checkBoxCameraLocal->isEnabled());
    ui->checkBoxStorageLocal->setChecked(m_user.perm_local_storage == PERM_STORAGE_ALL && ui->checkBoxStorageLocal->isEnabled());
    ui->checkBoxEventLocal->setChecked(m_user.perm_local_event == PERM_EVENT_ALL && ui->checkBoxEventLocal->isEnabled());
    ui->checkBoxSettingLocal->setChecked(m_user.perm_local_settings == PERM_SETTINGS_ALL && ui->checkBoxSettingLocal->isEnabled());
    ui->checkBoxStatusLocal->setChecked(m_user.perm_local_status == PERM_STATUS_ALL && ui->checkBoxStatusLocal->isEnabled());
    ui->checkBoxShutdownLocal->setChecked(m_user.perm_local_shutdown == PERM_SHUTDOWN_ALL && ui->checkBoxShutdownLocal->isEnabled());
    onLocalCheckBoxClicked(true);

    //remote
    if (m_user.perm_remote_live == PERM_LIVE_ALL) {
        on_checkBoxLiveViewRemote_clicked(true);
    } else if (m_user.perm_remote_live == PERM_LIVE_NONE) {
        on_checkBoxLiveViewRemote_clicked(false);
    } else {
        for (auto iter = m_liveViewRemoteMap.constBegin(); iter != m_liveViewRemoteMap.constEnd(); ++iter) {
            int permission = iter.key();
            QCheckBox *checkBox = iter.value();
            if (m_user.perm_remote_live & permission && checkBox->isEnabled()) {
                checkBox->setChecked(true);
            } else {
                checkBox->setChecked(false);
            }
        }
    }
    onLiveViewRemoteCheckBoxClicked(true);

    if (m_user.perm_remote_playback == PERM_PLAYBACK_ALL) {
        on_checkBoxPlaybackRemote_clicked(true);
    } else if (m_user.perm_remote_playback == PERM_PLAYBACK_NONE) {
        on_checkBoxPlaybackRemote_clicked(false);
    } else {
        for (auto iter = m_playbackRemoteMap.constBegin(); iter != m_playbackRemoteMap.constEnd(); ++iter) {
            int permission = iter.key();
            QCheckBox *checkBox = iter.value();
            if (m_user.perm_remote_playback & permission && checkBox->isEnabled()) {
                checkBox->setChecked(true);
            } else {
                checkBox->setChecked(false);
            }
        }
    }
    onPlaybackRemoteCheckBoxClicked(true);

    ui->checkBoxRetrieveRemote->setChecked(m_user.perm_remote_retrieve == PERM_RETRIEVE_ALL && ui->checkBoxRetrieveRemote->isEnabled());
    ui->checkBoxSmartAnalysisRemote->setChecked(m_user.perm_remote_smart == PERM_SMART_ALL && ui->checkBoxSmartAnalysisRemote->isEnabled());
    ui->checkBoxCameraRemote->setChecked(m_user.perm_remote_camera == PERM_CAMERA_ALL && ui->checkBoxCameraRemote->isEnabled());
    ui->checkBoxStorageRemote->setChecked(m_user.perm_remote_storage == PERM_STORAGE_ALL && ui->checkBoxStorageRemote->isEnabled());
    ui->checkBoxEventRemote->setChecked(m_user.perm_remote_event == PERM_EVENT_ALL && ui->checkBoxEventRemote->isEnabled());
    ui->checkBoxSettingRemote->setChecked(m_user.perm_remote_settings == PERM_SETTINGS_ALL && ui->checkBoxSettingRemote->isEnabled());
    ui->checkBoxStatusRemote->setChecked(m_user.perm_remote_status == PERM_STATUS_ALL && ui->checkBoxStatusRemote->isEnabled());
    ui->checkBoxShutdownRemote->setChecked(m_user.perm_remote_shutdown == PERM_SHUTDOWN_ALL && ui->checkBoxShutdownRemote->isEnabled());
    onRemoteCheckBoxClicked(true);
}

void EditUserLimitsDialog::saveLocalToProfile()
{
    if (ui->checkBoxLiveViewLocal->checkState() == Qt::Checked) {
        m_user.perm_local_live = PERM_LIVE_ALL;
    } else if (ui->checkBoxLiveViewLocal->checkState() == Qt::Unchecked) {
        m_user.perm_local_live = PERM_LIVE_NONE;
    } else {
        for (auto iter = m_liveViewLocalMap.constBegin(); iter != m_liveViewLocalMap.constEnd(); ++iter) {
            int permission = iter.key();
            int userPermission = m_user.perm_local_live;
            if (userPermission == -1){
                userPermission = 0;
            }
            QCheckBox *checkBox = iter.value();
            if (checkBox->isChecked()) {
                userPermission |= permission;
            } else {
                userPermission &= ~permission;
            }
            m_user.perm_local_live = static_cast<PERM_LIVE_E>(userPermission);
        }
    }

    if (ui->checkBoxPlaybackLocal->checkState() == Qt::Checked) {
        m_user.perm_local_playback = PERM_PLAYBACK_ALL;
    } else if (ui->checkBoxPlaybackLocal->checkState() == Qt::Unchecked) {
        m_user.perm_local_playback = PERM_PLAYBACK_NONE;
    } else {
        for (auto iter = m_playbackLocalMap.constBegin(); iter != m_playbackLocalMap.constEnd(); ++iter) {
            int permission = iter.key();
            int userPermission = m_user.perm_local_playback;
            if (userPermission == -1){
                userPermission = 0;
            }
            QCheckBox *checkBox = iter.value();
            if (checkBox->isChecked()) {
                userPermission |= permission;
            } else {
                userPermission &= ~permission;
            }
            m_user.perm_local_playback = static_cast<PERM_PLAYBACK_E>(userPermission);
        }
    }

    if (!qMsNvr->isSlaveMode()) {
        if (ui->checkBoxRetrieveLocal->isChecked()) {
            m_user.perm_local_retrieve = PERM_RETRIEVE_ALL;
        } else {
            m_user.perm_local_retrieve = PERM_RETRIEVE_NONE;
        }
        if (ui->checkBoxSmartAnalysisLocal->isChecked()) {
            m_user.perm_local_smart = PERM_SMART_ALL;
        } else {
            m_user.perm_local_smart = PERM_SMART_NONE;
        }
        if (ui->checkBoxCameraLocal->isChecked()) {
            m_user.perm_local_camera = PERM_CAMERA_ALL;
        } else {
            m_user.perm_local_camera = PERM_CAMERA_NONE;
        }
    }

    if (ui->checkBoxStorageLocal->isChecked()) {
        m_user.perm_local_storage = PERM_STORAGE_ALL;
    } else {
        m_user.perm_local_storage = PERM_STORAGE_NONE;
    }
    if (ui->checkBoxEventLocal->isChecked()) {
        m_user.perm_local_event = PERM_EVENT_ALL;
    } else {
        m_user.perm_local_event = PERM_EVENT_NONE;
    }
    if (ui->checkBoxSettingLocal->isChecked()) {
        m_user.perm_local_settings = PERM_SETTINGS_ALL;
    } else {
        m_user.perm_local_settings = PERM_SETTINGS_NONE;
    }
    if (ui->checkBoxStatusLocal->isChecked()) {
        m_user.perm_local_status = PERM_STATUS_ALL;
    } else {
        m_user.perm_local_status = PERM_STATUS_NONE;
    }
    if (ui->checkBoxShutdownLocal->isChecked()) {
        m_user.perm_local_shutdown = PERM_SHUTDOWN_ALL;
    } else {
        m_user.perm_local_shutdown = PERM_SHUTDOWN_NONE;
    }
}
void EditUserLimitsDialog::saveRemoteToProfile()
{
    if (ui->checkBoxLiveViewRemote->checkState() == Qt::Checked) {
        m_user.perm_remote_live = PERM_LIVE_ALL;
    } else if (ui->checkBoxLiveViewRemote->checkState() == Qt::Unchecked) {
        m_user.perm_remote_live = PERM_LIVE_NONE;
    } else {
        for (auto iter = m_liveViewRemoteMap.constBegin(); iter != m_liveViewRemoteMap.constEnd(); ++iter) {
            int permission = iter.key();
            int userPermission = m_user.perm_remote_live;
            if (userPermission == -1){
                userPermission = 0;
            }
            QCheckBox *checkBox = iter.value();
            if (checkBox->isChecked()) {
                userPermission |= permission;
            } else {
                userPermission &= ~permission;
            }
            m_user.perm_remote_live = static_cast<PERM_LIVE_E>(userPermission);
        }
    }

    if (ui->checkBoxPlaybackRemote->checkState() == Qt::Checked) {
        m_user.perm_remote_playback = PERM_PLAYBACK_ALL;
    } else if (ui->checkBoxPlaybackRemote->checkState() == Qt::Unchecked) {
        m_user.perm_remote_playback = PERM_PLAYBACK_NONE;
    } else {
        for (auto iter = m_playbackRemoteMap.constBegin(); iter != m_playbackRemoteMap.constEnd(); ++iter) {
            int permission = iter.key();
            int userPermission = m_user.perm_remote_playback;
            if (userPermission == -1){
                userPermission = 0;
            }
            QCheckBox *checkBox = iter.value();
            if (checkBox->isChecked()) {
                userPermission |= permission;
            } else {
                userPermission &= ~permission;
            }
            m_user.perm_remote_playback = static_cast<PERM_PLAYBACK_E>(userPermission);
        }
    }
    if (!qMsNvr->isSlaveMode()) {
        if (ui->checkBoxRetrieveRemote->isChecked()) {
            m_user.perm_remote_retrieve = PERM_RETRIEVE_ALL;
        } else {
            m_user.perm_remote_retrieve = PERM_RETRIEVE_NONE;
        }
        if (ui->checkBoxSmartAnalysisRemote->isChecked()) {
            m_user.perm_remote_smart = PERM_SMART_ALL;
        } else {
            m_user.perm_remote_smart = PERM_SMART_NONE;
        }
        if (ui->checkBoxCameraRemote->isChecked()) {
            m_user.perm_remote_camera = PERM_CAMERA_ALL;
        } else {
            m_user.perm_remote_camera = PERM_CAMERA_NONE;
        }
    }

    if (ui->checkBoxStorageRemote->isChecked()) {
        m_user.perm_remote_storage = PERM_STORAGE_ALL;
    } else {
        m_user.perm_remote_storage = PERM_STORAGE_NONE;
    }
    if (ui->checkBoxEventRemote->isChecked()) {
        m_user.perm_remote_event = PERM_EVENT_ALL;
    } else {
        m_user.perm_remote_event = PERM_EVENT_NONE;
    }
    if (ui->checkBoxSettingRemote->isChecked()) {
        m_user.perm_remote_settings = PERM_SETTINGS_ALL;
    } else {
        m_user.perm_remote_settings = PERM_SETTINGS_NONE;
    }
    if (ui->checkBoxStatusRemote->isChecked()) {
        m_user.perm_remote_status = PERM_STATUS_ALL;
    } else {
        m_user.perm_remote_status = PERM_STATUS_NONE;
    }
    if (ui->checkBoxShutdownRemote->isChecked()) {
        m_user.perm_remote_shutdown = PERM_SHUTDOWN_ALL;
    } else {
        m_user.perm_remote_shutdown = PERM_SHUTDOWN_NONE;
    }
}

void EditUserLimitsDialog::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("USER/142012", "User Permissions Edit"));
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

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

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

void EditUserLimitsDialog::onTabBarClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void EditUserLimitsDialog::onLocalCheckBoxClicked(bool checked)
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

void EditUserLimitsDialog::onRemoteCheckBoxClicked(bool checked)
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

void EditUserLimitsDialog::onLiveViewLocalCheckBoxClicked(bool checked)
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

void EditUserLimitsDialog::onLiveViewRemoteCheckBoxClicked(bool checked)
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

void EditUserLimitsDialog::onPlaybackLocalCheckBoxClicked(bool checked)
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

void EditUserLimitsDialog::onPlaybackRemoteCheckBoxClicked(bool checked)
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

void EditUserLimitsDialog::on_checkBoxAllLocal_clicked(bool checked)
{
    for (auto iter = m_localMap.constBegin(); iter != m_localMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checked && checkBox->isEnabled()) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }
    }
    on_checkBoxPlaybackLocal_clicked(checked);
    on_checkBoxLiveViewLocal_clicked(checked);
    if (checked) {
        ui->checkBoxAllLocal->setCheckState(Qt::Checked);
    }
}

void EditUserLimitsDialog::on_checkBoxAllRemote_clicked(bool checked)
{
    for (auto iter = m_remoteMap.constBegin(); iter != m_remoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checked && checkBox->isEnabled()) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }
    }
    on_checkBoxPlaybackRemote_clicked(checked);
    on_checkBoxLiveViewRemote_clicked(checked);
    if (checked) {
        ui->checkBoxAllRemote->setCheckState(Qt::Checked);
    }
}

void EditUserLimitsDialog::on_pushButton_ok_clicked()
{
    read_user_by_name(SQLITE_FILE_NAME, &m_user, m_user.username);
    saveLocalToProfile();
    saveRemoteToProfile();
    saveToProfileCameraConfiguration();

    accept();
}

void EditUserLimitsDialog::on_pushButton_cancel_clicked()
{
    reject();
}

void EditUserLimitsDialog::readFromProfileToDlgCameraConfiguration()
{
    //local liveview
    ui->checkBoxGroupLiveViewLocal->setCheckedFromString(QString(m_user.local_live_view_ex));

    //local playback
    ui->checkBoxGroupPlayBackLocal->setCheckedFromString(QString(m_user.local_playback_ex));

    //remote liveview
    ui->checkBoxGroupLiveViewRemote->setCheckedFromString(QString(m_user.remote_live_view_ex));

    //remote playback
    ui->checkBoxGroupPlayBackRemote->setCheckedFromString(QString(m_user.remote_playback_ex));
}

void EditUserLimitsDialog::saveToProfileCameraConfiguration()
{
    //local liveview
    QString localLiveviewMask = ui->checkBoxGroupLiveViewLocal->checkedMask();
    snprintf(m_user.local_live_view_ex, sizeof(m_user.local_live_view_ex), "%s", localLiveviewMask.toStdString().c_str());

    //local playback
    QString localPlaybackMask = ui->checkBoxGroupPlayBackLocal->checkedMask();
    snprintf(m_user.local_playback_ex, sizeof(m_user.local_playback_ex), "%s", localPlaybackMask.toStdString().c_str());

    //remote liveview
    QString remoteLiveviewMask = ui->checkBoxGroupLiveViewRemote->checkedMask();
    snprintf(m_user.remote_live_view_ex, sizeof(m_user.remote_live_view_ex), "%s", remoteLiveviewMask.toStdString().c_str());

    //remote playback
    QString remotePlaybackMask = ui->checkBoxGroupPlayBackRemote->checkedMask();
    snprintf(m_user.remote_playback_ex, sizeof(m_user.remote_playback_ex), "%s", remotePlaybackMask.toStdString().c_str());
}

void EditUserLimitsDialog::on_checkBoxLiveViewLocal_clicked(bool checked)
{
    for (auto iter = m_liveViewLocalMap.constBegin(); iter != m_liveViewLocalMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checked && checkBox->isEnabled()) {
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

void EditUserLimitsDialog::on_checkBoxLiveViewRemote_clicked(bool checked)
{
    for (auto iter = m_liveViewRemoteMap.constBegin(); iter != m_liveViewRemoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checked && checkBox->isEnabled()) {
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

void EditUserLimitsDialog::on_checkBoxPlaybackLocal_clicked(bool checked)
{
    for (auto iter = m_playbackLocalMap.constBegin(); iter != m_playbackLocalMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checked && checkBox->isEnabled()) {
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

void EditUserLimitsDialog::on_checkBoxPlaybackRemote_clicked(bool checked)
{
    for (auto iter = m_playbackRemoteMap.constBegin(); iter != m_playbackRemoteMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checked && checkBox->isEnabled()) {
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
