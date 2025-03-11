#ifndef ImagePageDayNightMulti_H
#define ImagePageDayNightMulti_H

#include "abstractimagepage.h"
#include <QMap>

extern "C" {
#include "msg.h"
}

namespace Ui {
class ImagePageDayNightMulti;
}
class DayNightSchedule;

class ImagePageDayNightMulti : public AbstractImagePage {
  Q_OBJECT

  enum smartIrType {
    m_pwm = 1 << 6,
    m_pwm_2 = 1 << 8,
    m_PWM_1IR_1W = 1 << 9,
    m_PWM_2IR = 1 << 10,
    m_PWM_2IR_1W = 1 << 11,
    m_PWM_1IR_2W = 1 << 23,
    m_PWM_2IR_1NW = 1 << 12,
    m_PWM_1IR = 1 << 14,
    m_PWM_3IR = 1 << 15,
    m_distance_measure = 1 << 20,
    m_fix_len = 1 << 21
  };

  enum TableColumn {
    ColumnCheck,
    ColumnId,
    ColumnTime,
    ColumnMinimumShutter,
    ColumnMaximumShutter,
    ColumnLimitGainLevel,
    ColumnIrCutLatency,
    ColumnIrCut,
    ColumnIrLed,
    ColumnColorMode,
    ColumnEdit
  };

public:
  explicit ImagePageDayNightMulti(QWidget *parent = nullptr);
  ~ImagePageDayNightMulti();

  void initializeData(int channel) override;
  void processMessage(MessageReceive *message) override;

private:
  void clearSettings();
  void setSettingsEnable(bool enable);

  void ON_RESPONSE_FLAG_GET_DAY_NIGHT_INFO(MessageReceive *message);
  void initDayNight();
  QString showTime(int id);
  QString showOffOnStatus(int type);
  QString showIRLedOffOnStatus(int type);
  QString showColorMode(int type);
  QString showShutter(int num);
  void deleteListAll();
  void saveDayNightSche();

protected:
  void resizeEvent(QResizeEvent *) override;

private slots:
  void closeWait();
  void slotEditDayNightInfo(int row, int column);
  void on_pushButton_copy_clicked();
  void on_pushButton_apply_clicked();
  void on_pushButton_back_clicked();
  void updateSche();

private:
  Ui::ImagePageDayNightMulti *ui;
  int m_channel;
  set_image_day_night_str m_info;
  QString m_sdkversion;
  QList<int> m_copyChannelList;
  int m_exposureCtrl = -1;
  int m_chipninterface = 0;
  bool m_isWhite = false;
  bool m_fullcolorSupport = false;
};

#endif // ImagePageDayNightMulti_H
