#ifndef DRAWITEMANPR_H
#define DRAWITEMANPR_H

#include "DrawItemMask.h"
#include <QGraphicsTextItem>

class DrawItemAnpr : public DrawItemMask
{
public:
    DrawItemAnpr(QGraphicsItem *parent = nullptr);

    void setIndex(int index);
    int index() const;

    void setName(const QString &name);
    QString name(bool jsonSupport);

    void updateSizeText();
    bool isText();
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int m_index = -1;
    QString m_name;

    QGraphicsTextItem *m_textItem = nullptr;
};

#endif // DRAWITEMANPR_H
