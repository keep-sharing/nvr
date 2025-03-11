#ifndef LOGWRITE_H
#define LOGWRITE_H

#include "MsObject.h"

extern "C" {
#include "log.h"
}

class LogWrite : public MsObject {
    Q_OBJECT

public:
    explicit LogWrite(QObject *parent = nullptr);

    static LogWrite *instance();

    void writeLog(int sub_type);

signals:

private:
    static LogWrite *self;
};

#endif // LOGWRITE_H
