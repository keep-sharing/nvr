#ifndef REGIONPEOPLECOUNTINGPOLYLINESCENE_H
#define REGIONPEOPLECOUNTINGPOLYLINESCENE_H

/******************************************************************
* @brief    IPC区域人数统计折线图
* @author   LiuHuanyu
* @date     2021-07-16
******************************************************************/

#include "LineScene.h"

class RegionPeopleCountingPolylineScene : public LineScene {
    Q_OBJECT

public:
    explicit RegionPeopleCountingPolylineScene(QObject *parent = nullptr);

protected:
    QString legendName(int channel) override;
};

#endif // REGIONPEOPLECOUNTINGPOLYLINESCENE_H
