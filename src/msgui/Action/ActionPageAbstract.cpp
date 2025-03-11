#include "ActionPageAbstract.h"
#include "ActionAbstract.h"
#include "MsLanguage.h"
#include "MyDebug.h"

extern "C" {
#include "msdb.h"
}

ActionPageAbstract::ActionPageAbstract(QWidget *parent)
    : QWidget(parent)
{
    m_action = qobject_cast<ActionAbstract *>(parent);
    if (!m_action) {
        qMsWarning() << "invalid parent:" << parent;
    }

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    QMetaObject::invokeMethod(this, "onLanguageChanged", Qt::QueuedConnection);
}

bool ActionPageAbstract::hasCache() const
{
    return m_cache;
}

void ActionPageAbstract::setCached()
{
    m_cache = true;
}

void ActionPageAbstract::clearCache()
{
    m_cache = false;
}

void ActionPageAbstract::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void ActionPageAbstract::showEvent(QShowEvent *event)
{
    loadData();
    QWidget::showEvent(event);
}

bool ActionPageAbstract::hasSchedule(schedule_day *schedule_day_array)
{
    bool has = false;
    for (int i(0); i < MAX_DAY_NUM; i++) {
        for (int j(0); j < MAX_WND_NUM * MAX_PLAN_NUM_PER_DAY; j++) {
            if ((QString(schedule_day_array[i].schedule_item[j].start_time) != "" && QString(schedule_day_array[i].schedule_item[j].start_time) != "00:00")
                || (QString(schedule_day_array[i].schedule_item[j].end_time) != "" && QString(schedule_day_array[i].schedule_item[j].end_time) != "00:00")) {
                has = true;
                break;
            }
        }
    }
    return has;
}

void ActionPageAbstract::onLanguageChanged()
{

}

void ActionPageAbstract::onCancelClicked()
{
    clearCache();
}
