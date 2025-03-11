#ifndef RUNNABLEHOLIDAYDATA_H
#define RUNNABLEHOLIDAYDATA_H

#include <QRunnable>
#include <QObject>

struct holiday;

class RunnableHolidayData : public QRunnable
{
public:
    RunnableHolidayData(holiday *data, QObject *obj, const QString &member);
    ~RunnableHolidayData() override;

    void run() override;

private:
    holiday *m_data = nullptr;
    QObject *m_obj = nullptr;
    QString m_member;
};

#endif // RUNNABLEHOLIDAYDATA_H
