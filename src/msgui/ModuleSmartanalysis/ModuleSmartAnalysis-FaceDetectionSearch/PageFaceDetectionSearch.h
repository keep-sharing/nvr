#ifndef PAGEFACEDETECTIONSEARCH_H
#define PAGEFACEDETECTIONSEARCH_H

#include "AbstractSettingPage.h"
#include "FaceBackupInfo.h"
#include "anpritemwidget.h"
#include "progressdialog.h"
#include <QDateTime>

extern "C" {
#include "msg.h"
}

namespace Ui {
class FaceDetectionSearch;
}

class PageFaceDetectionSearch : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PageFaceDetectionSearch(QWidget *parent = nullptr);
    ~PageFaceDetectionSearch();

    void initializeData() override;
    bool isCloseable() override;
    void closePage() override;

    NetworkResult dealNetworkCommond(const QString &commond) override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_EXPORT_FACE_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_FACE_BACKUP_BIGIMG(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(MessageReceive *message);
    //filter
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message);

    void showWait();
    void showVideoWait();
    void updateVideoInfo();
    void updateBigPicture();

    void stopCommonPlay();
    void stopAndClearCommonPlay();
    void searchFacePage(int page);

    int selectedItemIndex() const;
    void seekLivePlayback(const QDateTime &dateTime);

    void backupFile(const QList<int> &backupList);

    void clearTable();
    void updateTable(int index, const QImage &smallImage);

    void closeFaceSearch();

private slots:
    void onLanguageChanged() override;
    //progress
    void onSearchCanceled();
    void onChannelCheckBoxClicked();
    void onItemClicked(int index);
    void onItemChecked(int index, bool checked);

    void onRealPlaybackTime();
    void onSliderValueChanged(int value);

    void on_toolButtonStop_clicked();
    void on_toolButtonPlay_clicked();
    void on_toolButtonPrevious_clicked();
    void on_toolButtonNext_clicked();

    void on_checkBoxResultAll_clicked(bool checked);

    void on_toolButtonFirstPage_clicked();
    void on_toolButtonPreviousPage_clicked();
    void on_toolButtonNextPage_clicked();
    void on_toolButtonLastPage_clicked();
    void on_pushButtonGo_clicked();

    void on_pushButtonBackupAll_clicked();
    void on_pushButtonBackup_clicked();
    void on_pushButtonResultBack_clicked();

    //search page
    void on_pushButtonSearch_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::FaceDetectionSearch *ui;
    ProgressDialog *m_progress = nullptr;
    int m_progressSid = -1;

    int m_itemCountInPage = 20;
    QList<AnprItemWidget *> m_itemList;
    int m_selectedItemIndex = -1;
    QMap<int, FaceBackupInfo> m_faceMap;
    int m_allCount = 0;
    int m_searchedCount = 0;
    int m_faceSearchSid = -1;

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

#endif // PAGEFACEDETECTIONSEARCH_H
