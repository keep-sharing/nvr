#ifndef MYLINEEDITDIGITAL_H
#define MYLINEEDITDIGITAL_H

#include <QLineEdit>

class MyLineEditDigital : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEditDigital(QWidget *parent = nullptr);

    void setRange(int min, int max);
    void setAllowZero(bool enable);
    void setAllowEmpty(bool enable);

    void setText(const QString &text);

    int value();
    void setValue(const int &value);

signals:

private slots:
    void onEditingFinished();

private:
    int m_minValue = 0;
    int m_maxValue = 100;
    int m_currentValue = 0;
    bool m_allowZero = false;
    bool m_allowEmpty = false;
};

#endif // MYLINEEDITDIGITAL_H
