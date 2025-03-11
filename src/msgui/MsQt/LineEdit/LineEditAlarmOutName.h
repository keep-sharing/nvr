#ifndef LINEEDITALARMOUTNAME_H
#define LINEEDITALARMOUTNAME_H

#include "LineEdit.h"

class LineEditAlarmOutName : public LineEdit
{
    Q_OBJECT
public:
    explicit LineEditAlarmOutName(QWidget *parent = nullptr);

protected:
    virtual bool check() override;
    virtual QString tipString() override;

public slots:
};

#endif // LINEEDITALARMOUTNAME_H
