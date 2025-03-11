#ifndef PAGECOMMONBACKUP_H
#define PAGECOMMONBACKUP_H

#include "AbstractSettingPage.h"
#include "itembuttonwidget.h"
#include "progressdialog.h"
#include <QDateTime>

extern "C" {
#include "msg.h"
}

namespace Ui {
class CommonBackup;
}

class PageCommonBackup : public AbstractSettingPage {
    Q_OBJECT

public:
    enum BackupDisplayMode {
        ModeList,
        ModeChart
    };

    enum BackupColumn {
        ColumnCheck,
        ColumnChannel,
        ColumnDisk,
        ColumnTime,
        ColumnSize,
        ColumnPlay,
        ColumnLock
    };

    explicit PageCommonBackup(QWidget *parent = 0);
    ~PageCommonBackup();

    int StreamType = 0;

    void initializeData() override;
    void closePage() override;

    bool isCloseable() override;
    bool canAutoLogout() override;

    NetworkResult dealNetworkCommond(const QString &commond) override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_GET_REC_RANGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SEARCH_COM_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_COM_PICTURE(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_LOCK_COM_BACKUP(MessageReceive *message);
    //filter
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message);

    void resizeEvent(QResizeEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void updateTableList();
    void updateTableChart();
    void updateCheckedSize();
    void updateSelectedInfo();

    void closeBackupSearch();

    void openAudio(int channel);
    void closeAudio(int channel);

    void updateTimeRange();
    void seekLivePlayback(const QDateTime &dateTime);

    void immediatelyUpdatePlayTime();

private slots:
    void onLanguageChanged() override;

    void onChannelCheckBoxClicked();
    void on_comboBox_streamType_activated(int index);
    //table
    void onItemClicked(int row, int column);
    void onTableHeaderChecked(bool checked);
    //progress
    void onSearchCanceled();
    //
    void onRealPlaybackTime();
    void onSliderValueChanged(int value);

    void on_pushButton_search_clicked();
    void on_pushButton_back_clicked();

    //
    void onButtonGroupDisplayModeClicked(int id);

    //table chart
    void onTableChartClicked();

    void onToolButtonFirstPageClicked();
    void onToolButtonPreviousPageClicked();
    void onToolButtonNextPageClicked();
    void onToolButtonLastPageClicked();
    void onPushButtonGoClicked();

    void on_toolButton_stop_clicked();
    void on_toolButton_play_clicked();
    void on_toolButton_previous_clicked();
    void on_toolButton_next_clicked();

    void on_pushButton_backupAll_clicked();
    void on_pushButton_backup_clicked();
    void on_pushButton_back_2_clicked();

private:
    Ui::CommonBackup *ui;

    BackupDisplayMode m_displayMode = ModeList;

    QEventLoop m_eventLoop;
    ProgressDialog *m_progress = nullptr;

    QList<resp_search_common_backup> m_backupList;
    resp_search_common_backup m_selectedBackup;

    int m_pageIndex = 0;
    int m_pageCount = 0;

    int m_currentRow = -1;
    int m_playingSid = -1;
    int m_currentChannel = -1;

    int m_searchSid = -1;

    //play time
    QTimer *m_timer;
    QDateTime m_currentTime;
    QDateTime m_currentStartTime;
    QDateTime m_currentEndTime;

    //
    bool m_isAbaoutToQuit = false;
};

#endif // PAGECOMMONBACKUP_H
