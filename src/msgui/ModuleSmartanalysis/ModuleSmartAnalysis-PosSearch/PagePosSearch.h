#ifndef PAGEPOSSEARCH_H
#define PAGEPOSSEARCH_H

#include "AbstractSettingPage.h"
#include <QDateTime>
#include "PosTextEdit.h"
#include <QEventLoop>

extern "C" {
#include "msg.h"
}

class ProgressDialog;

namespace Ui {
class PosSearch;
}

class PagePosSearch : public AbstractSettingPage
{
    Q_OBJECT

    enum BackupColumn {
        ColumnCheck,
        ColumnPos,
        ColumnTime,
        ColumnInfo,
        ColumnPlay
    };

public:
    explicit PagePosSearch(QWidget *parent = nullptr);
    ~PagePosSearch();

    void initializeData() override;

    bool isCloseable() override;
    bool isChangeable() override;
    void closePage() override;
    bool canAutoLogout() override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SEARCH_POS_OPEN(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_POS_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_POS_DETAILS(MessageReceive *message);
    //
    void ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYPAUSE_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_COM_BACKUP_CLOSE(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYSEEK_BACKUP(MessageReceive *message);
    //
    void ON_RESPONSE_FLAG_PLAY_COM_PICTURE(MessageReceive *message);
    //
    void ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_POS_EXPORT(MessageReceive *message);
    //filter
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message);

    void updateBackupTable();
    void setCurrentBackupItem(int row);
    void updateBackupInfo();
    void clearCurrentInfo();

    void waitForSearchCommonBackup();
    void waitForPlayCommonBackup();
    void waitForPauseCommonBackup();
    void waitForResumeCommonBackup();
    void waitForStopCommonBackup();
    void waitForCloseCommonBackup();
    void waitForSeekCommonBackup(const QDateTime &dateTime);

    void waitForSearchCommonBackupPicture();

    void waitForSearchPosPage(int page);
    void waitForClosePosBackup();

    //
    void backupFile(const QList<int> &backupList);

private slots:
    void onLanguageChanged() override;

    //search
    void on_pushButtonSearch_clicked();
    void on_pushButtonBack_clicked();

    //table
    void onItemClicked(int row, int column);
    void onTableHeaderChecked(bool checked);
    //progress
    void onSearchCanceled();
    //
    void onRealPlaybackTime();
    void onSliderValueChanged(int value);
    //
    void on_toolButtonPos_clicked(bool checked);
    void on_toolButtonStop_clicked();
    void on_toolButtonPlay_clicked();
    void on_toolButtonPause_clicked();
    void on_toolButtonPrevious_clicked();
    void on_toolButtonNext_clicked();
    //page
    void onPageEdited(const QString &text);
    void on_toolButtonFirstPage_clicked();
    void on_toolButtonPreviousPage_clicked();
    void on_toolButtonNextPage_clicked();
    void on_toolButtonLastPage_clicked();
    void on_pushButtonGo_clicked();

    //
    void on_pushButtonBackupAll_clicked();
    void on_pushButtonBackup_clicked();
    void on_pushButtonResultBack_clicked();

private:
    Ui::PosSearch *ui;

    bool m_isAbaoutToQuit = false;

    ProgressDialog *m_progress = nullptr;
    int m_searchSid = -1;

    int m_allCount = 0;
    int m_backupSid = -1;
    QList<RespSearchPosBackup> m_posBackupList;
    QString m_detailPosText;

    int m_pageIndex = 0;
    int m_pageCount = 0;

    int m_currentRow = -1;

    QList<resp_search_common_backup> m_playBackupList;
    int m_streamType = -1;
    int m_playSid = -1;
    int m_playChannel = -1;
    //播放或暂停中
    bool m_isPlaying = false;

    //play time
    QTimer *m_timer = nullptr;
    QDateTime m_currentTime;
    QDateTime m_currentStartTime;
    QDateTime m_currentEndTime;

    //export
    bool m_isExporting = false;
    bool m_isExportCancel = false;
    QEventLoop m_waitForExport;
    QEventLoop m_waitForCheckDisk;
    int m_exportDiskPort = 0;
    int m_exportDiskFreeSize = 0;
};

#endif // PAGEPOSSEARCH_H
