#include "MultiScreenControl.h"
#include "MsDevice.h"
#include "SubControl.h"
#include "mainwindow.h"

MultiScreenControl::MultiScreenControl(QObject *parent)
    : QObject(parent)
{
    memset(&m_db_display, 0, sizeof(struct display));
    read_display(SQLITE_FILE_NAME, &m_db_display);
}

MultiScreenControl::~MultiScreenControl()
{
}

MultiScreenControl &MultiScreenControl::instance()
{
    static MultiScreenControl self;
    return self;
}

int MultiScreenControl::currentScreen() const
{
    return SubControl::instance()->currentScreen();
}

bool MultiScreenControl::isSubEnable() const
{
    return m_db_display.sub_enable;
}

/**
 * @brief MultiScreenControl::multiScreenSupport
 * @return 0: 不支持, 1: , 2:
 */
int MultiScreenControl::multiScreenSupport()
{
    if (qMsNvr->multiScreenSupport() == 2) {
        int homologous = get_param_int(SQLITE_FILE_NAME, PARAM_HOMOLOGOUS, 1);
        if (homologous == 0) {
            return 2;
        } else {
            return 3;
        }
    } else if (qMsNvr->multiScreenSupport() == 1) {
        return 1;
    } else {
        return 0;
    }
}

QRect MultiScreenControl::mainScreenGeometry() const
{
    return QRect(0, 0, 1920, 1080);
}

QRect MultiScreenControl::subScreenGeometry() const
{
    return QRect(1920, 0, 1920, 1080);
}
