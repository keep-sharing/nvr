#ifndef MASKWIDGET_H
#define MASKWIDGET_H

#include <QWidget>
#include <QMap>

class MaskWidget : public QWidget
{
    Q_OBJECT
public:
    enum OperationMode
    {
        ModeResizeLeft,     //改变一个item大小
        ModeResizeTopLeft,
        ModeResizeTop,
        ModeResizeTopRight,
        ModeResizeRight,
        ModeResizeBottomRight,
        ModeResizeBottom,
        ModeResizeBottomLeft,
        ModeNew,            //新建一个item
        ModeMove,           //移动一个item
        ModeNull
    };
    enum AnchorPosition
    {
        AnchorLeft,
        AnchorTopLeft,
        AnchorTop,
        AnchorTopRight,
        AnchorRight,
        AnchorBottomRight,
        AnchorBottom,
        AnchorBottomLeft
    };

    explicit MaskWidget(QWidget *parent = 0);

	QColor getColor();
    void setColor(const QColor &color);
    void setColorType(int type);
    void setSelected(bool selected);
    QRect realRect() const;
    void setRealRect(const QRect &rc);
    bool isUnderMouse(const QPoint &pos) const;

    int margin() const;
	void setIndex(int index);
	int getIndex();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

protected:
    void detectCursor(const QPoint &pos, bool setMode = false);

signals:

public slots:

protected:
    QColor m_color;
    int m_margin = 4;
    bool m_isSelected = false;
    QRect m_realRect;
    QRect m_tempGeometry;
    QMap<AnchorPosition, QRect> m_anchorMap;
    OperationMode m_operation = ModeNull;

    bool m_pressed = false;
    QPoint m_pressPoint;
    QPoint m_pressDistance;
	int m_index = -1;
};

#endif // MASKWIDGET_H
