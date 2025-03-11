#ifndef BASEITEMDELEGATE_H
#define BASEITEMDELEGATE_H

#include <QStyledItemDelegate>

extern const int ItemCheckedRole;  //bool
extern const int ItemChannelRole;  //int
extern const int ItemColorRole;    //QString, #FFFFFF
extern const int ItemTextRole;     //QString
extern const int ItemAddRole;    //bool

extern const int ItemEnabledRole;

class BaseItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BaseItemDelegate(QObject *parent = nullptr);

signals:

public slots:
};

#endif // BASEITEMDELEGATE_H
