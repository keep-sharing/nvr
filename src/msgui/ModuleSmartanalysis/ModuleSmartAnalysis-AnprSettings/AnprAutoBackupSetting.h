#ifndef ANPRAUTOBACKUPSETTING_H
#define ANPRAUTOBACKUPSETTING_H

#include "anprbasepage.h"
#include <QWidget>
#include "MsDevice.h"
#include <QEventLoop>
#include <QTimer>

struct resp_esata_backup_status;
struct resp_usb_info;
struct esata_auto_backup;

namespace Ui {
class AnprAutoBackupSetting;
}

class AnprAutoBackupSetting : public AnprBasePage
{
  Q_OBJECT

public:
  explicit AnprAutoBackupSetting(QWidget *parent = nullptr);
  ~AnprAutoBackupSetting();

  virtual void initializeData(int channel) override;
  void processMessage(MessageReceive *message) override;

private:
  void ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message);
  void ON_RESPONSE_FLAG_FORMAT_ESATA_DISK(MessageReceive *message);
  void ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(MessageReceive *message);
  void ON_RESPONSE_FLAG_FORMAT_EXPORT_DISK(MessageReceive *message);

  void ON_RESPONSE_FLAG_GET_ESATA_BACKUP_STATUS(MessageReceive *message);
  void ON_RESPONSE_FLAG_SET_ESATA_AUTO_BACKUP(MessageReceive *message);

  void esataUpdate();

  protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
  void onLanguageChanged();
  void setEnabled(bool enabled);

  void on_comboBoxAutoBackup_indexSet(int index);

  void on_pushButtonFormat_clicked();
  void on_pushButtonApply_clicked();
  void on_pushButtonBack_clicked();
  void onTimerout();

private:
  Ui::AnprAutoBackupSetting *ui;
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

#endif // ANPRAUTOBACKUPSETTING_H
