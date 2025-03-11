#include "abstractdiskedit.h"

AbstractDiskEdit::AbstractDiskEdit(QWidget *parent)
    : BaseShadowDialog(parent)
{
    m_waitting = new MsWaitting(this);
}

AbstractDiskEdit::~AbstractDiskEdit()
{
}

void AbstractDiskEdit::setDiskInfo(const BaseDiskInfo &info)
{
    m_diskInfo = info;
}
