#include "CameraPeopleCountingPolylineScene.h"
#include "MyDebug.h"
#include "LegendItem.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "pointlineitem.h"
#include <QPainter>
#include <qmath.h>

CameraPeopleCountingPolylineScene::CameraPeopleCountingPolylineScene(QObject *parent)
    : LineScene(parent)
{
    //IPC人数统计没有组的概念，这里为了兼容NVR人数统计的框架，默认为组0
    setCurrentGroup(0);
}

QString CameraPeopleCountingPolylineScene::legendName(int channel)
{
    QString text;
    switch (channel) {
    case -1:
        text = GET_TEXT("OCCUPANCY/74211", "Sum");
        break;
    case 0:
        text = GET_TEXT("OCCUPANCY/74209", "People Entered");
        break;
    case 1:
        text = GET_TEXT("OCCUPANCY/74210", "People Exited");
        break;
    }
    return text;
}
