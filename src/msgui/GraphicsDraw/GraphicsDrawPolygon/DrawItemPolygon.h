#ifndef DRAWITEMPOLYGON_H
#define DRAWITEMPOLYGON_H

#include <QGraphicsRectItem>
#include <QCursor>

class DrawScenePolygon;
class DrawItemPoint;

class DrawItem_Polygon : public QGraphicsRectItem
{
public:
    explicit DrawItem_Polygon(QGraphicsItem *parent = nullptr);

    static bool isLineCross(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4);
    static bool isLineContainsPoint(const QLineF &line, const QPointF &p);

    void setPoints(const QVector<QPointF> &pointVect);
    QVector<QPointF> points() const;
    QVector<QPointF> scenePoints() const;
    void clear();

    bool isConflict() const;
    //区域是否绘制完毕
    bool isFinished() const;
    bool isEmpty() const;

    void setConflict(bool value);
    bool checkConflict();

    void updateGeometry();

    void setSelected(bool selected);
    bool isSelected() const;

    void setEditable(bool enable);
    bool isEditable() const;

    void setPointSizeRange(int max, int min);

    /**************************************
     * 为了编辑时不显示报警和Stay Value
     * 点击Edit算开始编辑，提交到IPC认为时结束编辑
     *************************************/
    virtual void beginEdit();
    virtual void endEdit();
    virtual bool isEditting() const;

    //是否响应报警时颜色改变，配置界面可能不需要响应
    virtual void setAlarmable(bool enable);
    virtual bool isAlarmable() const;

    void setIndex(int newIndex);
    int index() const;

    DrawScenePolygon *scene() const;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    virtual QColor lineColor();

    virtual void showText();
    virtual void clearText();

    void clearPointItems();
    void addPointItem(const QPointF &p);

protected:
    int m_index = -1;
    bool m_isSelected = false;
    bool m_isEditable = false;
    bool m_isEditting = false;
    bool m_isAlarmable = true;

    bool m_isLeftPressed = false;
    bool m_isConflict = false;
    bool m_isFinished = false;
    bool m_isUnderMouse = false;
    int m_maxPointSize;
    int m_minPointSize;
    QPointF m_pressScenePos;
    QPointF m_startPos;
    QRectF m_startRc;
    QPointF m_hoverPos;
    QVector<DrawItemPoint *> m_pointItems;

    //
    QCursor m_cursorPen;
};

#endif // DRAWITEMPOLYGON_H
