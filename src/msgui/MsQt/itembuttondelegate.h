#ifndef ITEMBUTTONDELEGATE_H
#define ITEMBUTTONDELEGATE_H

#include <QStyledItemDelegate>

class ItemButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ItemButtonDelegate(const QPixmap &pixmap, QObject *parent);
    explicit ItemButtonDelegate(const QPixmap &pixmap, const QSize &size, QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QPixmap m_pixmap;
};

#endif // ITEMBUTTONDELEGATE_H
