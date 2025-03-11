#include "runnableplaybackrealtime.h"
#include <QDateTime>

extern "C" {

}

RunnablePlaybackRealTime::RunnablePlaybackRealTime(QObject *obj)
    : QRunnable()
    , m_obj(obj)
{
    setAutoDelete(false);
}

void RunnablePlaybackRealTime::run()
{

}
