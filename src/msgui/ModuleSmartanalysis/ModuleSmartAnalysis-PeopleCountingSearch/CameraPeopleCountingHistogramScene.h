#ifndef CAMERAPEOPLECOUNTINGHISTOGRAMSCENE_H
#define CAMERAPEOPLECOUNTINGHISTOGRAMSCENE_H

#include "HistogramScene.h"

class CameraPeopleCountingHistogramScene : public HistogramScene
{
    Q_OBJECT
public:
    explicit CameraPeopleCountingHistogramScene(QObject *parent = nullptr);

protected:
    QString legendName(int channel) override;
};

#endif // CAMERAPEOPLECOUNTINGHISTOGRAMSCENE_H
