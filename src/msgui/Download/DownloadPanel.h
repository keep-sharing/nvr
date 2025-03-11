#ifndef DOWNLOADPANEL_H
#define DOWNLOADPANEL_H

#include <QMap>
#include <QQueue>
#include "MsWidget.h"

extern "C" {
#include "msg.h"
}

class DownloadButton;
class DownloadItem;
class MessageReceive;

#define gDownload DownloadPanel::instance()
#define DOWNLOAD_BEGIN_ID 1000

namespace Ui {
class DownloadPanel;
}

class DownloadPanel : public MsWidget {
    Q_OBJECT

public:
    explicit DownloadPanel(QWidget *parent = 0);
    ~DownloadPanel();

    static DownloadPanel *instance();

    quint32 availableId();

    void appendItem(const req_auto_backup &backup);

    bool isDownloading() const;
    void stopDownload();

    void dealMessage(MessageReceive *message);
    void processMessage(MessageReceive *message) override;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void ON_RESPONSE_FLAG_ADD_AUTO_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE_EXPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_UPDATE_AUTO_BACKUP(MessageReceive *message);

    void updateItemWidgetInfo();
    void updateDownloadState();
    void updateRemainSec();
    void removeFromQueue(quint32 id);
    quint64 backupSize(const req_auto_backup &backup) const;

public slots:
    void onLanguageChanged();

private slots:
    void downloadNext();
    void onItemDeleted(quint32 id);

    void onTimerRemainTime();
    void onTimerSpeed();

    void on_toolButtonDelete_clicked();
    void on_toolButtonUDisk_clicked();

private:
    static DownloadPanel *self;

    Ui::DownloadPanel *ui;

    int m_marginLeft = 11;
    int m_marginTop = 31;
    int m_marginRight = 11;
    int m_marginBottom = 11;

    DownloadButton *m_download = nullptr;
    quint32 m_id = DOWNLOAD_BEGIN_ID;

    bool m_isDownloading = false;
    QQueue<req_auto_backup> m_backupQueue;
    req_auto_backup m_currentBackup;

    QMap<quint32, DownloadItem *> m_itemsMap;

    int m_remainSec = 0;
    QTimer *m_timerRemain = nullptr;

    int m_currentRemainSize = 0;
    int m_speed = 1024000;
    int m_speedSec = 0;
    QTimer *m_timerSpeed = nullptr;
};

#endif // DOWNLOADPANEL_H
