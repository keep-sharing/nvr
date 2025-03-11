#ifndef BACKUPTHUMB_H
#define BACKUPTHUMB_H

#include "MsWaitting.h"
#include "MsWidget.h"
#include "anpritemwidget.h"
#include <QEventLoop>
#include "CommonBackupControl.h"

extern "C" {
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class BackupThumb;
}

class BackupThumb : public MsWidget {
    Q_OBJECT

    enum BackupMode {
        ModeCommon,
        ModeEvent
    };

public:
    explicit BackupThumb(QWidget *parent = nullptr);
    ~BackupThumb();

    int StreamType = 0;

    void setCommonBackupList(const QList<resp_search_common_backup> &backup_list);
    void setEventBackupList(const QList<resp_search_event_backup> &backup_list);

    QList<resp_search_common_backup> checkedCommonBackupList() const;
    QList<resp_search_event_backup> checkedEventBackupList() const;
    resp_search_common_backup selectedCommonBakcup() const;
    resp_search_event_backup selectedEventBakcup() const;
    QImage selectedImage() const;

    void selectNext();
    void selectPrevious();

    void processMessage(MessageReceive *message);

    void setControl(CommonBackupControl *newControl);

    void setPageIndex(int pageIndex);

private:
    void ON_RESPONSE_FLAG_PLAY_COM_PICTURE(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_EVT_PICTURE(MessageReceive *message);

    void clearTable();
    void updateTable();
    void updateCheckedSize();

signals:
    void itemClicked();
    void stopPlay();

private slots:
    void onLanguageChanged();
    //
    void onItemClicked(int index);
    void onItemChecked(int index, bool checked);
    void on_checkBox_all_clicked(bool checked);

    void on_toolButton_firstPage_clicked();
    void on_toolButton_previousPage_clicked();
    void on_toolButton_nextPage_clicked();
    void on_toolButton_lastPage_clicked();

    void on_pushButton_go_clicked();

private:
    Ui::BackupThumb *ui;

    BackupMode m_mode = ModeCommon;

    QEventLoop m_eventLoop;
    MsWaitting *m_waitting = nullptr;

    int m_itemCountInPage = 20;
    QList<AnprItemWidget *> m_itemList;
    int m_selectedItemIndex = -1;

    quint64 m_allBytes = 0;
    int m_pageIndex = 0;
    int m_pageCount = 0;
    int m_allBackupCount = 0;
    int m_selectedBackupIndex = -1;
    QList<resp_search_common_backup> m_commonBackupList;
    QList<resp_search_event_backup> m_eventBackupList;

    QImage m_image;

    CommonBackupControl *m_control = nullptr;
    int m_showItemCount = 0;
};

#endif // BACKUPTHUMB_H
