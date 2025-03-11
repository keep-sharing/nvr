#ifndef TARGETINFO_H
#define TARGETINFO_H

#include <QImage>
#include <QString>

extern "C" {
#include "msdb.h"
#include "msg.h"
}

class TargetInfo {
public:
    enum TYPE {
        TARGET_NONE = 0,
        TARGET_ANPR = 0x0001,
        TARGET_VCA  = 0x0002,
        TARGET_FACE = 0x0004
    };

    explicit TargetInfo();
    virtual ~TargetInfo();

    void makeImage();

    TYPE type() const;
    virtual int channel() const = 0;
    virtual QString timeString() const = 0;
    QImage smallImage() const;

protected:
    TYPE m_type = TARGET_NONE;
    QImage m_smallImage;
    QByteArray m_smallImageData;
};

#endif // TARGETINFO_H
