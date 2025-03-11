#ifndef LINEEDITDIGIT_H
#define LINEEDITDIGIT_H

/******************************************************************
* @brief    纯数字输入框
* @author   LiuHuanyu
* @date     2021-08-09
******************************************************************/

#include "LineEdit.h"

class LineEditDigit : public LineEdit {
    Q_OBJECT

public:
    explicit LineEditDigit(QWidget *parent = nullptr);

    void setRange(int min, int max);

protected:
    virtual bool check() override;
    virtual QString tipString() override;

private:
    int m_minValue = 0;
    int m_maxValue = 0;
};

#endif // LINEEDITDIGIT_H
