#ifndef ZOOMCONTROL_H
#define ZOOMCONTROL_H

#include <QWidget>

class ZoomControl : public QWidget
{
    Q_OBJECT
public:
    explicit ZoomControl(QWidget *parent = nullptr);

    int nvrWidth() const;
    int nvrHeight() const;

    void clearZoom();

protected:
    void showEvent(QShowEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:
    void sendZoom();

signals:
    void zoomChanged(int zoom, QRect rc);

private:
    bool m_isPressed = false;
    QPoint m_pressDistance;

    //100-1000
    int m_zoom = 100;
    qreal m_scale = 1;

    QSize m_nvrScreenSize;
    QSize m_qtScreenSize;
};

#endif // ZOOMCONTROL_H
