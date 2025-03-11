#ifndef PAGEEVENTBACKUP_H
#define PAGEEVENTBACKUP_H

#include "AbstractSettingPage.h"
#include "itembuttonwidget.h"
#include "progressdialog.h"
#include <QDateTime>
#include "CommonBackupControl.h"

extern "C" {
#include "msg.h"
}

namespace Ui {
class EventBackup;
}

class PageEventBackup : public AbstractSettingPage {
    Q_OBJECT

public:
    enum BackupDisplayMode {
        ModeList,
        ModeChart
    };

    enum BackupColumn {
        ColumnCheck,
        ColumnSource,
        ColumnChannel,
        ColumnDisk,
        ColumnTime,
        ColumnPlay
    };

    explicit PageEventBackup(QWidget *parent = 0);
    ~PageEventBackup();

    int StreamType = 0;

    void initializeData() override;
    void closePage() override;

    bool isCloseable() override;
    bool canAutoLogout() override;

    NetworkResult dealNetworkCommond(const QString &commond) override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_SEARCH_EVT_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SEARCH_EVT_PAGE(MessageReceive *message);
    //filter
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message);

    void resizeEvent(QResizeEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void updateTableList();
    void updateTableChart();
    void updateCheckedSize();
    void updateSelectedInfo();

    void initializeVcaSubType();
    void initializeSmartSubType();

    void closeBackupSearch();

    void openAudio(int channel);
    void closeAudio(int channel);
    void immediatelyUpdatePlayTime();

    void stopPlay();

private slots:
    void onLanguageChanged() override;

    void onChannelCheckBoxClicked();
    //table
    void onItemClicked(int row, int column);
    void onTableHeaderChecked(bool checked);
    //progress
    void onSearchCanceled();
    //
    void onRealPlaybackTime();
    void onSliderValueChanged(int value);

    void on_comboBox_mainType_activated(int index);
    void on_comboBox_subType_indexSet(int index);

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

    void on_toolButtonStop_clicked();
    void on_toolButtonPlay_clicked();
    void on_toolButtonPause_clicked();
    void on_toolButtonPrevious_clicked();
    void on_toolButtonNext_clicked();

    void on_pushButton_backupAll_clicked();
    void on_pushButton_backup_clicked();
    void on_pushButton_back_2_clicked();

private:
    Ui::EventBackup *ui;

    BackupDisplayMode m_displayMode = ModeList;

    QEventLoop m_eventLoop;
    ProgressDialog *m_progress = nullptr;

    QList<resp_search_event_backup> m_backupList;
    resp_search_event_backup m_selectedBackup;

    int m_pageIndex = 0;
    int m_pageCount = 0;

    int m_currentRow = -1;
    int m_playingSid = -1;
    int m_currentChannel = -1;

    int m_searchSid = -1;

    //play time
    QTimer *m_timer = nullptr;
    QDateTime m_currentTime;
    QDateTime m_currentStartTime;
    QDateTime m_currentEndTime;

    //
    bool m_isAbaoutToQuit = false;

    CommonBackupControl *m_control = nullptr;
};

#endif // PAGEEVENTBACKUP_H
