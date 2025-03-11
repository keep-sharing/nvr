#ifndef REGIONPEOPLECOUNTINGHISTOGRAMSCENE_H
#define REGIONPEOPLECOUNTINGHISTOGRAMSCENE_H

#include "HistogramScene.h"

class RegionPeopleCountingHistogramScene : public HistogramScene
{
    Q_OBJECT
public:
    explicit RegionPeopleCountingHistogramScene(QObject *parent = nullptr);

protected:
    QString legendName(int channel) override;
};

#endif // REGIONPEOPLECOUNTINGHISTOGRAMSCENE_H
