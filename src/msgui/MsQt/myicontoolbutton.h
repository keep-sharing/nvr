#ifndef MYICONTOOLBUTTON_H
#define MYICONTOOLBUTTON_H

#include "mytoolbutton.h"

class MyIconToolButton : public MyToolButton
{
    Q_OBJECT
public:
    enum State
    {
        StateNone,
        StateNormal,
        StateChecked,
        StateDisabled
    };

    explicit MyIconToolButton(QWidget *parent = nullptr);

    void setNormalIcon(const QString &icon);
    void setCheckedIcon(const QString &icon);
    void setDisabledIcon(const QString &icon);

    void setIsNeedToolTip(bool isNeedToolTip);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:

private:
    State m_state = StateNone;
    QIcon m_iconNormal;
    QIcon m_iconChecked;
    QIcon m_iconDisabled;
    bool m_isNeedToolTip = false;
};

#endif // MYICONTOOLBUTTON_H
