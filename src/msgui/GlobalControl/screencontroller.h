#pragma once
#include <QObject>
#include "msdefs.h"

class ScreenControllerPrivate;
class ScreenController : public QObject
{
    Q_OBJECT
public:
    ScreenController();
    ~ScreenController();

    static ScreenController *instance();

    void prepare();
    void blackScreen(int mili = 250);
    void setMouseSpeed(qreal level);
    void setRefreshRate(DisplayDcMode_e);
    int refreshRate();
    void speedUp();
    void speedDown();
    qreal mouseRealAccel();

private:
    void changeMouseSpeed();
    ScreenControllerPrivate *d = nullptr;
};
