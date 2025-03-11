#ifndef PTZSCHEDULETASKEDIT_H
#define PTZSCHEDULETASKEDIT_H

#include "BaseShadowDialog.h"
#include "scheduletime.h"

class MyTimeRangeEdit;

namespace Ui {
class PtzScheduleTaskEdit;
}

class PtzScheduleTaskEdit : public BaseShadowDialog
{
  Q_OBJECT

public:
  explicit PtzScheduleTaskEdit(QWidget *parent = nullptr);
  ~PtzScheduleTaskEdit();
  void setScheduleArray(uint16_t** scheduleArray, bool supportWiper, bool supportSpeedDome);
  uint16_t** scheduleArray() const;

private:
  void showTime();
  bool saveSetting();

signals:
  void editingFinished();

private slots:
  void onLanguageChanged();

  void on_comboBox_day_activated(int index);
  void on_comboBox_type_activated(int index);
  void on_comboBoxNo_activated(int index);

  void onTimeEditingFinished(int beginMinutes, int endMinutes);
  void onTimeCleared(int beginMinutes, int endMinutes);

  void on_pushButton_copy_clicked();
  void on_pushButton_ok_clicked();
  void on_pushButton_cancel_clicked();

private:
  Ui::PtzScheduleTaskEdit *ui;
  QList<MyTimeRangeEdit *> m_timeEditList;

  uint16_t **m_scheduleArray;
  bool m_supportWiper = false;
  bool m_supportSpeedDome = false;

};

#endif // PTZSCHEDULETASKEDIT_H
