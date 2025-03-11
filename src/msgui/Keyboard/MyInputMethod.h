#ifndef MYINPUTMETHOD_H
#define MYINPUTMETHOD_H

#include <QWSInputMethod>
#include "Keyboard.h"

class MyInputMethod : public QWSInputMethod
{
    Q_OBJECT

public:
    MyInputMethod();

    void updateHandler(int type) override;
    void setKeyboard(QWidget *keyboard);

private:
    void showKeyboard(const QPoint &pos);
    void hideKeyboard();

private:
    Keyboard *m_keyboard = nullptr;
};

#endif // MYINPUTMETHOD_H
