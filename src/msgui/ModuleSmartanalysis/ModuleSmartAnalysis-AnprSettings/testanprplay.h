#ifndef TESTANPRPLAY_H
#define TESTANPRPLAY_H

#include "BaseShadowDialog.h"

namespace Ui {
class TestAnprPlay;
}

class TestAnprPlay : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit TestAnprPlay(QWidget *parent = 0);
    ~TestAnprPlay();

    void setPlayRect(const QRect &rc);
    QRect playRect() const;

private:
    Ui::TestAnprPlay *ui;
};

#endif // TESTANPRPLAY_H
