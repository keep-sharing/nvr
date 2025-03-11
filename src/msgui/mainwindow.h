#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "LiveView.h"
#include <QMainWindow>

class QFileSystemWatcher;
class EventPopup;
class MyFileSystemDialog;
class LiveView;
class LiveViewOccupancyManager;
class MessageBox;
class MessagePool;
class MsDevice;
class MsDisk;
class MsLogin;
class MsUser;
class MsWizard;
class PlaybackWindow;
class SettingContent;
class SubControl;
class TestHardware;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static MainWindow *s_mainWindow;
    static MainWindow *instance();

    void initializeData();
    void showLiveView(LiveView::ShowReason reason = LiveView::NormalShowReason);
    void hideLiveView();
    void showLogin();
    void showWizard();

    //菜单
    void dealMouseMove(const QPoint &pos);

    void readyToQuit();

signals:
    void liveViewVisibilityChanged(bool);

protected:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void paintEvent(QPaintEvent *) override;

private:
    void initPlayback();
    void releasePlayback();

    void initSettingContent();

    void ON_RESPONSE_FLAG_POWER_SHORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_POWER_LONG(MessageReceive *message);
    void ON_RESPONSE_FLAG_ENABLE_SCREEN_WEB(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_DISPARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_USER_UPDATE(MessageReceive *message);

public slots:
    void logout();
    void logoutToLogin();
    void resetGeometry();

    void dealMessage();
    void menuItemClicked(const MainMenu::MenuItem &item);
    void enterMenuLayout();

    //
    void onReadyToQuit();
    void onCoreStoped();

private slots:
    void checkDisk();

    void onFocusWidgetChanged(QWidget *old, QWidget *now);
    void onDirectoryChanged(const QString &path);

    //
    void onPermissionUpdateCheck();
    //wizard
    void onWizardFinished();
    //login
    void onLoginFinished();
    //第一次进入提示格式化
    void onDetectedUninitializedDisk();
    //playback
    void onPlaybackClosed();
    //settings
    void onSettingsClosed();

    void resumeLiveView();

    void onShutdownClicked();

private:
    Ui::MainWindow *ui;

    QFileSystemWatcher *m_fileWatcher = nullptr;
    bool m_mouseConnected = false;

    MessagePool *m_messagePool = nullptr;
    LiveView *m_liveView = nullptr;
    PlaybackWindow *m_playback = nullptr;
    SettingContent *m_settingContent = nullptr;

    EventPopup *m_eventPopup = nullptr;
    //login
    MsLogin *m_login = nullptr;
    //wizard
    MsWizard *m_wizard = nullptr;
    //
    MsDevice *m_msDevice = nullptr;
    //
    SubControl *m_subControl = nullptr;
    //
    MyFileSystemDialog *m_fileSystemDialog = nullptr;
    NetworkCommond *m_networkCommond = nullptr;

    MessageBox *m_shutdownMessageBox = nullptr;

    TestHardware *m_testHardware = nullptr;

    bool m_isFakerShutdown = false;
};

#endif // MAINWINDOW_H
