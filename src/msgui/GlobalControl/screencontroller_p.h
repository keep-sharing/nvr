#pragma once
#include <QObject>
#include <QScreen>
#include <QBasicTimer>

class ScreenController;
class ScreenControllerPrivate : public QObject
{
    Q_OBJECT
public:
    ScreenControllerPrivate(QObject *parent);
    Q_INVOKABLE void onChangeMouseSpeed();
    void setScreenCallback(QScreen *);
protected:
    void timerEvent(QTimerEvent *);
public slots:
    void initializeTde();
    void afterBlackScreen();
private:
    void setFbVisible(int fb, bool visible);
    void tdeScaling(int x, int y, int width, int height);
    void waitForVSync(int fb);
    int refreshRate = 30;
    bool isTdeInitialized = false;
    QBasicTimer screenUpdater;
    qreal mouseSpeed = 2;
    qreal mouseRealSpeed = 0;
    ScreenController *q = nullptr;
    friend ScreenController;
};
