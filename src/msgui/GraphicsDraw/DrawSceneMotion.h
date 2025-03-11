#ifndef DRAWSCENEMOTION_H
#define DRAWSCENEMOTION_H

#include <QGraphicsScene>
#include "DrawItemMotion.h"

class DrawSceneMotion : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit DrawSceneMotion(QObject *parent = nullptr);

    void clearAll();
    void selectAll();

    void setRegion(char *region);
    void getRegion(char *region);

    void setObjectSize(int value);

signals:

private slots:
    void onSceneRectChanged(const QRectF &rect);

private:
    DrawItemMotion *m_motionItem = nullptr;
};

#endif // DRAWSCENEMOTION_H
