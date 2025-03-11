#ifndef CAMERAPEOPLECOUNTINGPOLYLINESCENE_H
#define CAMERAPEOPLECOUNTINGPOLYLINESCENE_H

/******************************************************************
* @brief    IPC人数统计折线图
* @author   LiuHuanyu
* @date     2021-07-16
******************************************************************/

#include "LineScene.h"

class CameraPeopleCountingPolylineScene : public LineScene {
    Q_OBJECT

public:
    explicit CameraPeopleCountingPolylineScene(QObject *parent = nullptr);

protected:
    QString legendName(int channel) override;

private:
};

#endif // CAMERAPEOPLECOUNTINGPOLYLINESCENE_H
