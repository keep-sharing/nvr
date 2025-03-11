#ifndef ABSTRACTDISKEDIT_H
#define ABSTRACTDISKEDIT_H

#include "basediskinfo.h"
#include "BaseShadowDialog.h"
#include "MsWaitting.h"

class AbstractDiskEdit : public BaseShadowDialog {
    Q_OBJECT

public:
    AbstractDiskEdit(QWidget *parent = nullptr);
    virtual ~AbstractDiskEdit();

    virtual void setDiskInfo(const BaseDiskInfo &info);

protected:
    BaseDiskInfo m_diskInfo;

    MsWaitting *m_waitting = nullptr;
};

#endif // ABSTRACTDISKEDIT_H
