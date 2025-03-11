#include "myspinbox.h"
#include <QLineEdit>
#include "MyDebug.h"

MySpinBox::MySpinBox(QWidget *parent) : QSpinBox(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);

    QLineEdit *edit = lineEdit();
    connect(edit, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited(QString)));
    connect(edit, SIGNAL(editingFinished()), this, SLOT(onValueExceed()));
}

void MySpinBox::setAutoDetectionRange(bool enable)
{
    m_isAutoDetect = enable;
}
void MySpinBox::onTextEdited(const QString &text)
{
    if (m_isAutoDetect)
    {
        int value = text.toInt();
        int minValue = minimum();
        if (value < minimum())
        {
            setValue(minValue);
        }
    }
}
/**
* @description      设置spinBox的最大值,当超过时自动设置为最大值
* @date                 2021.03.02
* @author:              Kirin
*/
void MySpinBox::setAutoAmendments(bool enable, int maxNum)
{
    m_isAutoAmendments = enable;
    m_maxNum = maxNum;
}
void MySpinBox::onValueExceed()
{
    QLineEdit *edit = lineEdit();
    if (m_isAutoAmendments) {
        int value = edit->text().trimmed().toInt();
        if (value > m_maxNum) {
            setValue(m_maxNum);
        }
    }
}
