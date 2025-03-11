#ifndef MESSAGESEND_H
#define MESSAGESEND_H

#include "osa_typedef.h"

class MessageSend
{
public:
    MessageSend();

    int sendto = 0;
    int type = 0;
    int size = 0;
    char pSmallData[1024]; //for data real size <= 1024
    char *pBigData = nullptr; //for data real size > 1024
    UInt64 obj = 0;

    void clear();
};

#endif // MESSAGESEND_H
