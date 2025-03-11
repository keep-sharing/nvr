#ifndef PAGEANPRSEARCH_H
#define PAGEANPRSEARCH_H

#include "AnprBackupInfo.h"
#include "AbstractSettingPage.h"
#include "anpritemwidget.h"
#include "progressdialog.h"
#include <QDateTime>
#include <QEventLoop>

extern "C" {
#include "msg.h"
}

struct resp_search_anpr_backup;

namespace Ui {
class AnprAnalysis;
}

class PageAnprSearch : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PageAnprSearch(QWidget *parent = nullptr);
    ~PageAnprSearch();

    void initializeData() override;
    bool isCloseable() override;
    void closePage() override;
    bool canAutoLogout() override;

    NetworkResult dealNetworkCommond(const QString &commond) override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_ANPR_BACKUP_BIGIMG(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_COM_PICTURE(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_EXPORT_ANPR_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message);
    //filter
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message);

    void showWait();
    void showVideoWait();

    void clearTable();
    void updateTable(int index, const QImage &smallImage);
    void updateVideoInfo();
    void updateBigPicture();

    void searchAnprPage(int page);
    void closeBackup();
    void closeAnprSearch();

    void searchCommonPicture();
    void stopCommonPlay();
    void stopAndClearCommonPlay();

    int selectedItemIndex() const;

    QString directionString(int value) const;

    void backupFile(const QList<int> &backupList);

    void seekLivePlayback(const QDateTime &dateTime);

private slots:
    void onLanguageChanged() override;

    void onChannelCheckBoxClicked();
    //progress
    void onSearchCanceled();

    void on_pushButton_search_clicked();
    void on_pushButton_back_clicked();

    //
    void onItemClicked(int index);
    void onItemChecked(int index, bool checked);
    void on_checkBox_all_clicked(bool checked);

    void onRealPlaybackTime();
    void onSliderValueChanged(int value);

    void on_toolButton_stop_clicked();
    void on_toolButton_play_clicked();
    void on_toolButton_previous_clicked();
    void on_toolButton_next_clicked();

    void onPageEdited(const QString &text);
    void on_toolButton_firstPage_clicked();
    void on_toolButton_previousPage_clicked();
    void on_toolButton_nextPage_clicked();
    void on_toolButton_lastPage_clicked();
    void on_pushButton_go_clicked();

    void on_pushButton_backupAll_clicked();
    void on_pushButton_backup_clicked();
    void on_pushButton_back_2_clicked();

    void on_comboBoxSpeed_currentIndexChanged(int index);

private:
    Ui::AnprAnalysis *ui;

    ProgressDialog *m_progress = nullptr;
    int m_progressSid = -1;

    int m_itemCountInPage = 20;
    QList<AnprItemWidget *> m_itemList;
    int m_selectedItemIndex = -1;
    QMap<int, AnprBackupInfo> m_anprMap;
    int m_allCount = 0;
    int m_searchedCount = 0;
    int m_anprSearchSid = -1;

    int m_pageIndex = 0;
    int m_pageCount = 0;

    int m_currentIndex = -1;
    int m_currentBackupSid = -1;
    int m_currentChannel = -1;
    int m_playSid = -1;

    //play time
    QTimer *m_timer = nullptr;
    QDateTime m_currentTime;
    QDateTime m_currentStartTime;
    QDateTime m_currentEndTime;

    QList<resp_search_common_backup> m_backupList;
    QEventLoop m_waitForSearch;
    int m_currentFileType = FILE_TYPE_MAIN;

    //export
    bool m_isExporting = false;
    bool m_isExportCancel = false;
    QEventLoop m_waitForExport;
    QEventLoop m_waitForStopPlay;
    QEventLoop m_waitForCheckDisk;
    int m_exportDiskPort = 0;
    int m_exportDiskFreeSize = 0;
    //
    bool m_isAbaoutToQuit = false;
    bool m_isAboutToShowMessage = false;
};

#endif // PAGEANPRSEARCH_H
