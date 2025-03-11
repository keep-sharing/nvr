#include "RunnableGetDatabaseCamera.h"
#include "MyDebug.h"

extern "C" {
#include "msdb.h"
}

RunnableGetDatabaseCamera::RunnableGetDatabaseCamera(QObject *obj, const QString &member, int channel)
    : QRunnable()
    , m_obj(obj)
    , m_member(member)
    , m_channel(channel)
{
    qMsDebug();
}

void RunnableGetDatabaseCamera::run()
{
    camera cam;
    memset(&cam, 0, sizeof(camera));
    read_camera(SQLITE_FILE_NAME, &cam, m_channel);

    QMetaObject::invokeMethod(m_obj, m_member.toLocal8Bit().data(), Qt::QueuedConnection, Q_ARG(camera, cam));
}
