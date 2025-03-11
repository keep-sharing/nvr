#ifndef HEADERVIEW_H
#define HEADERVIEW_H

#include <QHeaderView>

class HeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit HeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    void setCheckable(bool checkable);
    void setCheckState(Qt::CheckState state);

    void setSortableForColumn(int column, int enable);
    bool sortableForColumn(int column);

protected:
    void paintSection ( QPainter * painter, const QRect & rect, int logicalIndex ) const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void headerChecked(bool checked);

private:
    QPoint m_pressedPoint;

    bool m_checkable = false;
    Qt::CheckState m_checkState = Qt::Unchecked;
    QPixmap m_uncheckedPixmap;
    QPixmap m_checkedPixmap;
    QPixmap m_partiallycheckedPixmap;

    QPixmap m_uncheckedDisablePixmap;
    QPixmap m_checkedDisablePixmap;
    QPixmap m_partiallycheckedDisablePixmap;

    QMap<int, bool> m_sortableMap;
};

#endif // HEADERVIEW_H
