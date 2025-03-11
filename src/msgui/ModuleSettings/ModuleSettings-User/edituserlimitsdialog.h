#ifndef EDITUSERLIMITSDIALOG_H
#define EDITUSERLIMITSDIALOG_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
}

class QCheckBox;

enum privilegeTabIndex {
    INDEX_BASIC_PERMISSION = 0,
    INDEX_CAMERA_CONFIGURATION
};

enum PermType {
    PERM_LIVE ,
    PERM_PLAYBACK,
    PERM_RETRIEVE,
    PERM_SMART,
    PERM_CAMERA,
    PERM_STORAGE,
    PERM_EVENT,
    PERM_SETTINGS,
    PERM_STATUS,
    PERM_SHUTDOWN
};

namespace Ui {
class EditUserLimitsDialog;
}

class EditUserLimitsDialog : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit EditUserLimitsDialog(QWidget *parent = 0);
    ~EditUserLimitsDialog();

    void setEditUserLimitInit(const db_user &user);
    void getEditUserLimitInit(db_user &user);

private:
    void initializeLocalMap();
    void initializeRemoteMap();

    void readFromProfileToDlg();
    void saveLocalToProfile();
    void saveRemoteToProfile();

    void readFromProfileToDlgCameraConfiguration();
    void saveToProfileCameraConfiguration();

public slots:
    void onLanguageChanged();

private slots:
    void onTabBarClicked(int index);

    void onLocalCheckBoxClicked(bool checked);
    void onRemoteCheckBoxClicked(bool checked);

    void onLiveViewLocalCheckBoxClicked(bool checked);
    void onLiveViewRemoteCheckBoxClicked(bool checked);
    void onPlaybackLocalCheckBoxClicked(bool checked);
    void onPlaybackRemoteCheckBoxClicked(bool checked);

    void on_checkBoxAllLocal_clicked(bool checked);
    void on_checkBoxAllRemote_clicked(bool checked);

    void on_checkBoxLiveViewLocal_clicked(bool checked);
    void on_checkBoxLiveViewRemote_clicked(bool checked);
    void on_checkBoxPlaybackLocal_clicked(bool checked);
    void on_checkBoxPlaybackRemote_clicked(bool checked);

    void on_pushButton_cancel_clicked();
    void on_pushButton_ok_clicked();

private:
    Ui::EditUserLimitsDialog *ui;

    QCheckBox *camCheckBoxLocalLiveview[MAX_CAMERA];
    QCheckBox *camCheckBoxLocalPlayback[MAX_CAMERA];
    QCheckBox *camCheckBoxRemoteLiveview[MAX_CAMERA];
    QCheckBox *camCheckBoxRemotePlayback[MAX_CAMERA];

    db_user m_user;

    QMap<int, QCheckBox *> m_localMap;
    QMap<int, QCheckBox *> m_liveViewLocalMap;
    QMap<int, QCheckBox *> m_playbackLocalMap;
    QMap<int, QCheckBox *> m_remoteMap;
    QMap<int, QCheckBox *> m_liveViewRemoteMap;
    QMap<int, QCheckBox *> m_playbackRemoteMap;
};

#endif // EDITUSERLIMITSDIALOG_H
