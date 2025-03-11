#ifndef OPTIMALSPLICINGDISTANCE_H
#define OPTIMALSPLICINGDISTANCE_H

#include "BaseDialog.h"
#include <QTimer>

class CentralMessage;

namespace Ui {
class OptimalSplicingDistance;
}

class OptimalSplicingDistance : public BaseDialog {
  Q_OBJECT

public:
  explicit OptimalSplicingDistance(QWidget *parent = 0);
  ~OptimalSplicingDistance();

  void showImageInfo(int channel, const QRect &videoGeometry);

  void processMessage(MessageReceive *message);

protected:
  void ON_RESPONSE_FLAG_GET_IPC_IMAGE_SPLICEDISTANCE(MessageReceive *message);

  void showEvent(QShowEvent *event) override;
  bool eventFilter(QObject *object, QEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

  void escapePressed() override;

  bool isMoveToCenter() override;
  bool isAddToVisibleList() override;

private slots:
  void onLanguageChanged();

  void onSetImageInfo();
  void onSendTimer();
  void onSliderChange(int value);
  void onDoubleSpinBoxChange(double value);

  void on_pushButton_default_clicked();
  void on_pushButton_close_clicked();

private:
  Ui::OptimalSplicingDistance *ui;

  int m_currentChannel = -1;
  bool m_sendChange = true;

  QTimer *m_sendTimer;

  bool m_titlePressed = false;
  QPoint m_titlePressedDistance;
};

#endif // OPTIMALSPLICINGDISTANCE_H
