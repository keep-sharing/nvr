#ifndef DRAWITEMOBJECTBOX_H
#define DRAWITEMOBJECTBOX_H

#include <QGraphicsRectItem>

extern "C" {
#include "msg.h"
}

namespace ObjectBox {
enum VCA_OBJECT_E {
    VCA_OBJECT_Unknown = 0, /**< Unknown */
    VCA_OBJECT_Person, /**< Human/Person */
    VCA_OBJECT_Vehicle, /**< Car/Vehicle */
    VCA_OBJECT_NUM, /**< Total Class */
};

enum VCA_EVENT_E {
    VCA_REGION_ENTRANCE = 0,
    VCA_REGION_EXITING,
    VCA_ADVANCED_MOTION,
    VCA_TAMPER_DEFOCUS,
    VCA_LINE_CROSSING,
    VCA_OBJECT_LOITERING,
    VCA_HUMAN_DETECTION,
    VCA_PEOPLE_COUNTING,
    VCA_OBJECT_LEFT_REMOVED,
    VCA_EVENT_TYPE_MAX_NUM,

};
}

class DrawItemObjectBox : public QGraphicsRectItem {

    enum Mode {
        ModeNone,
        ModeVca,
        ModeRegionPeopleCounting
    };

public:
    DrawItemObjectBox(QGraphicsItem *parent = nullptr);
    ~DrawItemObjectBox() override;

    void showBox(int id, int type, int alarm);
    void showBox(const RegionalRectBox &box);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    void drawVca(QPainter *painter);
    void drawRegionalPeopleCounting(QPainter *painter);

private:
    Mode m_mode = ModeNone;
    int m_id = 0;
    int m_class = 0;
    int m_alarm = 0;

    RegionalRectBox m_regionRectInfo;
};

#endif // DRAWITEMOBJECTBOX_H
