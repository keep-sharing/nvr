#ifndef ITEMCHANNELDELEGATE_H
#define ITEMCHANNELDELEGATE_H

#include "baseitemdelegate.h"

class ItemChannelDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ItemChannelDelegate(QObject *parent = nullptr);

    void setCheckedPixmap(const QString &text);
    void setUncheckedPixmap(const QString &text);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:

public slots:

private:
    QPixmap m_uncheckedPixmap;
    QPixmap m_checkedPixmap;
};

#endif // ITEMCHANNELDELEGATE_H
