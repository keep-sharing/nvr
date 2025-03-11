#include "MyDoubleSpinBox.h"
#include <QLineEdit>
#include "MyDebug.h"

MyDoubleSpinBox::MyDoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
  setContextMenuPolicy(Qt::NoContextMenu);

  QLineEdit *edit = lineEdit();
  connect(edit, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited(QString)));
  connect(edit, SIGNAL(editingFinished()), this, SLOT(onValueExceed()));
}

void MyDoubleSpinBox::setAutoDetectionRange(bool enable)
{
  m_isAutoDetect = enable;
}
void MyDoubleSpinBox::onTextEdited(const QString &text)
{
  if (m_isAutoDetect)
  {
    double value = text.toDouble();
    double minValue = minimum();
    if (value < minimum())
    {
      setValue(minValue);
    }
  }
}
/**
* @description      设置DoubleSpinBox的最大值,当超过时自动设置为最大值
* @date                 2021.03.02
* @author:              Kirin
*/
void MyDoubleSpinBox::setAutoAmendments(bool enable, double maxNum)
{
  m_isAutoAmendments = enable;
  m_maxNum = maxNum;
}
void MyDoubleSpinBox::onValueExceed()
{
  QLineEdit *edit = lineEdit();
  if (m_isAutoAmendments) {
    double value = edit->text().trimmed().toDouble();
    if (value > m_maxNum) {
      setValue(m_maxNum);
    }
  }
}
