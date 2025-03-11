#ifndef MSMESSAGE_H
#define MSMESSAGE_H

#include "centralmessage.h"
#include <QObject>

class MsMessage : public QObject {
    Q_OBJECT
public:
    explicit MsMessage(QObject *parent = nullptr);

signals:
};

#endif // MSMESSAGE_H
