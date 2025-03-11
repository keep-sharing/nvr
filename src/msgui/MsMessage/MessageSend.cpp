#include "MessageSend.h"

MessageSend::MessageSend()
{
}

void MessageSend::clear()
{
    if (pBigData) {
        delete[] pBigData;
        pBigData = nullptr;
    }
    size = 0;
}
