#ifndef AUTOBACKUP_H
#define AUTOBACKUP_H

#include "AbstractSettingPage.h"
#include "MsDevice.h"
#include <QEventLoop>
#include <QTimer>
#include <QWidget>

struct resp_esata_backup_status;
struct resp_usb_info;
struct esata_auto_backup;

namespace Ui {
class AutoBackup;
}

class AutoBackup : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit AutoBackup(QWidget *parent = nullptr);
    ~AutoBackup();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message);
    void ON_RESPONSE_FLAG_FORMAT_ESATA_DISK(MessageReceive *message);
    void ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(MessageReceive *message);
    void ON_RESPONSE_FLAG_FORMAT_EXPORT_DISK(MessageReceive *message);

    void ON_RESPONSE_FLAG_GET_ESATA_BACKUP_STATUS(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_ESATA_AUTO_BACKUP(MessageReceive *message);

    void esataUpdate();
    void updateAvailable();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onLanguageChanged() override;
    void setEnabled(bool enabled);

    void on_comboBoxAutoBackup_indexSet(int index);

    void on_pushButtonFormat_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

    void on_comboBoxStarageDevice_indexSet(int index);
    void onTimerout();

private:
    Ui::AutoBackup *ui;

    QEventLoop m_eventLoop;

    resp_usb_info *m_esata_usb = nullptr;
    QMap<int, resp_usb_info> m_nasInfoMap;
    QMap<int, resp_usb_info> m_usbInfoMap;
    //all
    QMap<int, resp_usb_info> m_diskMap;
    bool m_isFormatting = false;

    esata_auto_backup *m_esata_backup = nullptr;
    resp_esata_backup_status *m_backup_status = nullptr;
    QTimer *m_timer = nullptr;
};

#endif // AUTOBACKUP_H
