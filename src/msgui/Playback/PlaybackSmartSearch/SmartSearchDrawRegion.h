#ifndef SMARTSEARCHDRAWREGION_H
#define SMARTSEARCHDRAWREGION_H

#include "drawmotion.h"

class SmartSearchDrawRegion : public DrawMotion {
    Q_OBJECT

public:
    explicit SmartSearchDrawRegion(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
};

#endif // SMARTSEARCHDRAWREGION_H
