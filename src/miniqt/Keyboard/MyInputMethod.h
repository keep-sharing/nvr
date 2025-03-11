#ifndef MYINPUTMETHOD_H
#define MYINPUTMETHOD_H

#include "KeyboardWidget.h"
#include <QWSInputMethod>

class MyInputMethod : public QWSInputMethod {
    Q_OBJECT

public:
    MyInputMethod();

    void updateHandler(int type) override;
    void setKeyboard(QWidget *keyboard);

private:
    void showKeyboard(const QPoint &pos);
    void hideKeyboard();

private:
    KeyboardWidget *m_widgetKeyboard = nullptr;
};

#endif // MYINPUTMETHOD_H
