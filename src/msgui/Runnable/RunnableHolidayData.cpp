#include "RunnableHolidayData.h"

extern "C" {
#include "msdb.h"
#include "msdefs.h"
}

RunnableHolidayData::RunnableHolidayData(holiday *data, QObject *obj, const QString &member)
    : QRunnable()
    , m_obj(obj)
    , m_member(member)
{
    m_data = new holiday[MAX_HOLIDAY];
    memcpy(m_data, data, sizeof(struct holiday) * MAX_HOLIDAY);
}

RunnableHolidayData::~RunnableHolidayData()
{
    delete[] m_data;
}

void RunnableHolidayData::run()
{
    for (int i = 0; i < MAX_HOLIDAY; ++i) {
        holiday &holidayInfo = m_data[i];
        write_holiday(SQLITE_FILE_NAME, &holidayInfo);
    }
    QMetaObject::invokeMethod(m_obj, m_member.toLocal8Bit().data(), Qt::QueuedConnection);
}
