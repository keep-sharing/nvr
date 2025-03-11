#include "CameraPeopleCountingHistogramScene.h"
#include "MsLanguage.h"

CameraPeopleCountingHistogramScene::CameraPeopleCountingHistogramScene(QObject *parent)
    : HistogramScene(parent)
{
    //IPC人数统计没有组的概念，这里为了兼容NVR人数统计的框架，默认为组0
    setCurrentGroup(0);
}

QString CameraPeopleCountingHistogramScene::legendName(int channel)
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
