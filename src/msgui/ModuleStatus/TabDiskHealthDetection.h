#ifndef TABDISKHEALTHDETECTION_H
#define TABDISKHEALTHDETECTION_H

#include "AbstractSettingTab.h"
#include "DrawSceneDiskHealth.h"

class MsWaitting;
class QTimer;
class DrawView;

namespace Ui {
class TabDiskHealthDetection;
}

class TabDiskHealthDetection : public AbstractSettingTab {

  Q_OBJECT

public:
  explicit TabDiskHealthDetection(QWidget *parent = nullptr);
  ~TabDiskHealthDetection();

  void initializeData() override;

  void processMessage(MessageReceive *message) override;
  void filterMessage(MessageReceive *message) override;
  void refreshData();
  void clearInfo();
  void saveData();
protected:
  void ON_RESPONSE_FLAG_GET_DISK_HEALTH_MANAGEMENT_DATA(MessageReceive *message);
  void ON_RESPONSE_FLAG_GET_DISK_HEALTH_MANAGEMENT_LOG(MessageReceive *message);

  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;

private slots:
  void onLanguageChanged() override;
  void on_comboBoxDisk_activated(int index);

  void on_pushButtonExport_clicked();
  void onTimer();
  void on_checkBoxEnable_clicked(bool checked);

private:
  Ui::TabDiskHealthDetection *ui;
  MsWaitting *m_waitting;
  QTimer *m_timer;
  DrawView *m_drawView = nullptr;
  DrawSceneDiskHealth *m_drawScene = nullptr;
  int m_requestCount = 0;
  int m_HMStatus = 0;
  bool m_supportHM = 0;
  bool m_hasDisk = 0;

};

#endif // TABDISKHEALTHDETECTION_H
