#include "RegionPeopleCountingPolylineScene.h"
#include "MsLanguage.h"

RegionPeopleCountingPolylineScene::RegionPeopleCountingPolylineScene(QObject *parent)
    : LineScene(parent)
{
    //IPC人数统计没有组的概念，这里为了兼容NVR人数统计的框架，默认为组0
    setCurrentGroup(0);
}

QString RegionPeopleCountingPolylineScene::legendName(int channel)
{
    QString text;
    switch (channel) {
    case -1:
        text = GET_TEXT("OCCUPANCY/74213", "Total");
        break;
    case 0:
        text = QString("%1 1").arg(GET_TEXT("PEOPLECOUNTING_SEARCH/140003", "Region"));
        break;
    case 1:
        text = QString("%1 2").arg(GET_TEXT("PEOPLECOUNTING_SEARCH/140003", "Region"));
        break;
    case 2:
        text = QString("%1 3").arg(GET_TEXT("PEOPLECOUNTING_SEARCH/140003", "Region"));
        break;
    case 3:
        text = QString("%1 4").arg(GET_TEXT("PEOPLECOUNTING_SEARCH/140003", "Region"));
        break;
    }
    return text;
}
