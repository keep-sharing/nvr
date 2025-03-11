#include "runnablesavelayout.h"
#include "MyDebug.h"

extern "C" {
#include "msdb.h"
}

RunnableSaveLayout::RunnableSaveLayout(layout_custom *layout)
    : QRunnable()
{
    m_layout = new layout_custom;
    memset(m_layout, 0, sizeof(layout_custom));
    memcpy(m_layout, layout, sizeof(layout_custom));

    //兼容数据库存储单引号错误
    QString amendName = QString(m_layout->name).replace("'", "''");
    snprintf(m_layout->name, sizeof(m_layout->name), "%s", amendName.toStdString().c_str());

    if (QString(m_layout->name).isEmpty() || (m_layout->screen < 0) || (m_layout->screen > 1)) {
        qMsWarning() << QString("name: %1, screen: %2, type: %3, page: %4")
                            .arg(m_layout->name).arg(m_layout->screen).arg(m_layout->type).arg(m_layout->page);
    }
}

RunnableSaveLayout::~RunnableSaveLayout()
{
    if (m_layout) {
        delete m_layout;
        m_layout = nullptr;
    }
}

void RunnableSaveLayout::run()
{
    write_layout_custom_64Screen(SQLITE_FILE_NAME, m_layout);
}
