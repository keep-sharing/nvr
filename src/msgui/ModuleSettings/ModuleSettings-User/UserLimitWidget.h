#ifndef USERLIMITWIDGET_H
#define USERLIMITWIDGET_H

#include <QCheckBox>
#include <QMap>
#include <QWidget>

extern "C" {
#include "msdb.h"
#include "msdefs.h"
}

namespace Ui {
class UserLimitWidget;
}
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
class UserLimitWidget : public QWidget {
    Q_OBJECT

public:
    explicit UserLimitWidget(QWidget *parent = nullptr);
    ~UserLimitWidget();

    void initializeUserLimit(const db_user &user);
    void setReadOnly(bool readOnly);
    void onLanguageChanged();

protected:
    void showEvent(QShowEvent *) override;

private:
    void initializeLocalMap();
    void initializeRemoteMap();

    void readBasicPermission(const db_user &user);
    void readCameraConfiguration(const db_user &user);

private slots:
    void onTabBarClicked(int index);

    void onLocalCheckBoxClicked(bool checked);
    void onRemoteCheckBoxClicked(bool checked);
    void onLiveViewLocalCheckBoxClicked(bool checked);
    void onLiveViewRemoteCheckBoxClicked(bool checked);
    void onPlaybackLocalCheckBoxClicked(bool checked);
    void onPlaybackRemoteCheckBoxClicked(bool checked);


    void on_checkBoxLiveViewLocal_clicked(bool checked);
    void on_checkBoxLiveViewRemote_clicked(bool checked);
    void on_checkBoxPlaybackLocal_clicked(bool checked);
    void on_checkBoxPlaybackRemote_clicked(bool checked);


private:
    Ui::UserLimitWidget *ui;

    QMap<int, QCheckBox *> m_localMap;
    QMap<int, QCheckBox *> m_liveViewLocalMap;
    QMap<int, QCheckBox *> m_playbackLocalMap;
    QMap<int, QCheckBox *> m_remoteMap;
    QMap<int, QCheckBox *> m_liveViewRemoteMap;
    QMap<int, QCheckBox *> m_playbackRemoteMap;
};

#endif // USERLIMITWIDGET_H
