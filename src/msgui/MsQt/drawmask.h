#ifndef DRAWMASK_H
#define DRAWMASK_H

#include <QWidget>

extern "C" {
#include "msg.h"
}

class MaskWidget;

class DrawMask : public QWidget {
    Q_OBJECT
public:
    enum OperationMode {
        ModeNew, //新建一个item
        ModeOld, //操作旧的一个item
        ModeNull
    };

    explicit DrawMask(QWidget *parent = 0);

    void setColor(const QColor &color);
    void setColorType(int type);
    void clearSelected();
    void clearAll();

    void setMaxMaskCount(int count);

    void addMask(const mask_area_ex &area);
    void getMask(mask_area_ex *area_array, int count = MAX_MASK_AREA_NUM);
    void setMaskEnable(int id, bool enable);
    void setMaskMaxCount(int count);
    void hideAll();
    void showAll();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    MaskWidget *maskUnderMouse(const QPoint &pos) const;

    virtual MaskWidget *makeNewWidget();

signals:

public slots:

protected:
    bool m_pressed = false;
    QPoint m_pressPoint;

    int m_colorType;
    QColor m_selectedColor;

    int m_maxMaskCount = MAX_MASK_AREA_NUM;
    MaskWidget *m_currentMask = nullptr;
    QList<MaskWidget *> m_maskList;
    OperationMode m_operation = ModeNull;
};

#endif // DRAWMASK_H
