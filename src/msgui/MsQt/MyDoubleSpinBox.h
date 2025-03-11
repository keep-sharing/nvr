#ifndef MYDOUBLESPINBOX_H
#define MYDOUBLESPINBOX_H

#include <QDoubleSpinBox>

class MyDoubleSpinBox : public QDoubleSpinBox
{
  Q_OBJECT
public:
  explicit MyDoubleSpinBox(QWidget *parent = 0);

  void setAutoDetectionRange(bool enable);
  void setAutoAmendments(bool enable, double maxNum);

signals:


private slots:
  void onTextEdited(const QString &text);
  void onValueExceed();
private:
  bool m_isAutoDetect = false;
  bool m_isAutoAmendments = false;
  double m_maxNum = 0;
};

#endif // MYDOUBLESPINBOX_H
