#ifndef PAGEPICTUREBACKUP_H
#define PAGEPICTUREBACKUP_H

#include "AbstractSettingPage.h"
#include "itembuttonwidget.h"
#include "progressdialog.h"
#include <QDateTime>

extern "C" {
#include "msg.h"
}

namespace Ui {
class PictureBackup;
}

class PagePictureBackup : public AbstractSettingPage {
    Q_OBJECT

public:
    enum BackupColumn {
        ColumnCheck,
        ColumnChannel,
        ColumnDisk,
        ColumnTime,
        ColumnSize
    };

    explicit PagePictureBackup(QWidget *parent = 0);
    ~PagePictureBackup();

    void initializeData() override;
    void closePage() override;

    bool isCloseable() override;
    bool canAutoLogout() override;

    NetworkResult dealNetworkCommond(const QString &commond) override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_SEARCH_PIC_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SEARCH_PIC_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_PIC_BACKUP(MessageReceive *message);
    //filter
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE(MessageReceive *message);

    void resizeEvent(QResizeEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void updateTable();
    void updateSelectedSize();
    void updateSelectedInfo();

    void searchPicturePage(int page);
    void closeBackup();

    void openAudio(int channel);
    void closeAudio(int channel);
    //    void tertiaryTypeInit(int index);

private slots:
    void onLanguageChanged() override;

    //table
    void onItemClicked(int row, int column);
    void onTableHeaderChecked(bool checked);
    //progress
    void onSearchCanceled();

    void on_pushButton_search_clicked();
    void on_pushButton_back_clicked();

    void on_toolButton_firstPage_clicked();
    void on_toolButton_previousPage_clicked();
    void on_toolButton_nextPage_clicked();
    void on_toolButton_lastPage_clicked();
    void on_pushButton_go_clicked();

    void on_toolButton_previous_clicked();
    void on_toolButton_next_clicked();

    void on_pushButton_backupAll_clicked();
    void on_pushButton_backup_clicked();
    void on_pushButton_back_2_clicked();

    void on_comboBox_pictureType_activated(int index);
    //    void on_comboBox_subType_activated(int index);

private:
    Ui::PictureBackup *ui;

    ProgressDialog *m_progress = nullptr;

    QList<resp_search_picture_backup> m_backupList;
    QEventLoop m_eventLoop;

    int m_pageIndex = 0;
    int m_pageCount = 0;

    int m_currentRow = -1;
    int m_currentChannel = -1;

    int m_searchSid = -1;
};

#endif // PAGEPICTUREBACKUP_H
