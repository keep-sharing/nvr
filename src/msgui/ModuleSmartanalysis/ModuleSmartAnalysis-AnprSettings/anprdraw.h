#ifndef ANPRDRAW_H
#define ANPRDRAW_H

#include <QWidget>

extern "C" {
#include "msg.h"
}

class AnprRegion;

class AnprDraw : public QWidget {
    Q_OBJECT
public:
    enum OperationMode {
        ModeNew, //新建一个item
        ModeOld, //操作旧的一个item
        ModeNull
    };

    explicit AnprDraw(QWidget *parent = nullptr);

    void finishEdit();

    QString regionName(int index) const;
    void setRegionName(int index, const QString &name);

    void clearAt(int index);
    void clearCurrent();
    void clearSelected();
    void clearAll();

    bool hasDraw() const;
    bool isFull() const;
    void setCurrentIndex(int index);

    void setRegionData(ms_lpr_position_info *region_array);
    void getRegionData(ms_lpr_position_info *region_array);
    void setBaseRegionResolution(int width, int height);

protected:
    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    AnprRegion *regionUnderMouse(const QPoint &pos) const;
    int availableIndex() const;

signals:

public slots:

protected:
    int m_maxRegion = 4;

    bool m_pressed = false;
    QPoint m_pressPoint;

    int m_currentIndex = -1;
    AnprRegion *m_selectedRegion = nullptr;
    AnprRegion *m_editingRegion = nullptr;

    QList<AnprRegion *> m_regionList;
    OperationMode m_operation = ModeNull;

    int m_baseRegionWidth = 0;
    int m_baseRegionHeight = 0;
};

#endif // ANPRDRAW_H
