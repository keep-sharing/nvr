#ifndef VIDEOOBJECT_H
#define VIDEOOBJECT_H

#include <QMap>
#include <QWidget>

extern "C" {
#include "msg.h"
}

class VideoObject : public QWidget {
    Q_OBJECT

    enum Position {
        None,
        Left,
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Move
    };

public:
    enum ObjectMode {
        ModeNone,
        ModeMin,
        ModeMax
    };

    explicit VideoObject(QWidget *parent = 0);

    void setObjectInfo(ms_vca_settings_info *info);
    void getObjectInfo(ms_vca_settings_info *info);
    void getRealObjectInfo(ms_vca_settings_info *info);
    void setRealObjectInfo(ms_vca_settings_info *info);
    void getVideoRectInfo(ms_video_rect_info *info);

    void setMinObject();
    void setMaxObject();
    void saveObject();

    void clearObject();
    bool isVaild(const ObjectMode &mode);

    int realWidth();
    int realHeight();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *) override;

private:
    QPointF lineCenter(const QLineF &line);
    void updateHandleRect();
    void setMouseCursor(const QPoint &point);

    QPoint physicalPos(int x, int y);
    QPoint logicalPos(const QPoint &pos);
    qreal physicalWidth(qreal w);
    qreal physicalHeight(qreal h);

public:
    qreal logicalWidth(qreal w);
    qreal logicalHeight(qreal h);

signals:
    void sigDrawup();
    void sigDrawup(bool isSizeChanged, bool isPosChanged);
public slots:

private:
    bool m_pressed = false;
    QPoint m_pressPoint;
    QPoint m_tempRectPos;

    Position m_position = None;
    QMap<Position, QRect> m_handleMap;
    QRect m_objectRect;
    QRect m_minObject;
    QRect m_maxObject;

    int m_realWidth = 0;
    int m_realHeight = 0;

    ObjectMode m_mode = ModeNone;
};

#endif // VIDEOOBJECT_H
