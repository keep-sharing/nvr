#ifndef MYSPINBOX_H
#define MYSPINBOX_H

#include <QSpinBox>

class MySpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit MySpinBox(QWidget *parent = 0);

    void setAutoDetectionRange(bool enable);
    void setAutoAmendments(bool enable, int maxNum);

signals:


private slots:
    void onTextEdited(const QString &text);
    void onValueExceed();
private:
    bool m_isAutoDetect = false;
    bool m_isAutoAmendments = false;
    int m_maxNum = 0;
};

#endif // MYSPINBOX_H
