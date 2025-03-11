#ifndef PLAYBACKFILEMANAGEMENT_H
#define PLAYBACKFILEMANAGEMENT_H

#include "BaseShadowDialog.h"
#include <QDateTime>

class MessageReceive;

struct resp_search_common_backup;

namespace Ui {
class PlaybackFileManagement;
}

class PlaybackFileManagement : public BaseShadowDialog {
    Q_OBJECT

public:
    enum Mode {
        ModeVideo,
        ModeSnapshot,
        ModeLock,
        ModeTag,
        ModeNone
    };

    explicit PlaybackFileManagement(QWidget *parent = nullptr);
    ~PlaybackFileManagement();

    static PlaybackFileManagement *s_fileManagement;
    static PlaybackFileManagement *instance();

    void close();

    bool hasFreeSid();

    void cleanup();

    bool hasNotExportedVideoFile() const;
    bool hasNotExportedPictureFile() const;

    //截图
    int waitForSnapshot(int winid);
    //剪切
    int waitForCutPlayback(const QDateTime &begin, const QDateTime &end);
    //锁定
    int addLockFile(const resp_search_common_backup &common_backup);
    int waitForPlaybackLockAll(const QDateTime &dateTime);
    int waitForPlaybackLock(const QDateTime &dateTime);
    //标签
    int waitForTagAll(const QMap<int, QDateTime> &dateTimeMap, const QString &name = QString());
    int waitForTag(const QDateTime &dateTime, const QString &name = QString());

    void initializeData();

    void setCurrentMode(PlaybackFileManagement::Mode mode);

    void dealMessage(MessageReceive *message);

private slots:
    void onLanguageChanged();
    void onTabClicked(int index);

    void on_pushButton_exportAll_clicked();
    void on_pushButton_export_clicked();
    void onPushButtonCancelClicked();
    void on_pushButton_deleteAll_clicked();
    void on_pushButton_delete_clicked();

private:
    Ui::PlaybackFileManagement *ui;

    Mode m_mode = ModeNone;
};

#endif // PLAYBACKFILEMANAGEMENT_H
