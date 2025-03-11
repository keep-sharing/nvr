#include "LogWrite.h"
#include "centralmessage.h"
#include "mainwindow.h"

LogWrite *LogWrite::self = nullptr;

LogWrite::LogWrite(QObject *parent)
    : MsObject(parent)
{
}

LogWrite *LogWrite::instance()
{
    if (!self) {
        self = new LogWrite(MainWindow::instance());
    }
    return self;
}

void LogWrite::writeLog(int sub_type)
{
    struct log_data log;
    memset(&log, 0, sizeof(struct log_data));

    if (SUB_EVENT_MIN <= sub_type && sub_type <= SUB_EVENT_MAX) {
        log.log_data_info.mainType = MAIN_EVENT;
    } else if (SUB_OP_LOCAL_MIN <= sub_type && sub_type <= SUB_OP_REMOTE_MAX) {
        log.log_data_info.mainType = MAIN_OP;
    } else if (SUB_EXCEPT_MIN <= sub_type && sub_type <= SUB_EXCEPT_MAX) {
        log.log_data_info.mainType = MAIN_EXCEPT;
    } else if (SUB_INFO_MIN <= sub_type && sub_type <= SUB_INFO_MAX) {
        log.log_data_info.mainType = MAIN_INFO;
    }

    log.log_data_info.subType = sub_type;
    log.log_data_info.parameter_type = SUB_PARAM_NONE;

    sendMessageOnly(REQUEST_FLAG_LOG_WRITE, &log, sizeof(struct log_data));
}
