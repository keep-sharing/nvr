#include "baseitemdelegate.h"

const int ItemCheckedRole = Qt::UserRole + 20;  //bool
const int ItemChannelRole = Qt::UserRole + 21;  //int
const int ItemColorRole = Qt::UserRole + 22;    //QString, #FFFFFF
const int ItemTextRole = Qt::UserRole + 23;     //QString
const int ItemAddRole = Qt::UserRole + 24;     //bool

const int ItemEnabledRole = Qt::UserRole + 30;

BaseItemDelegate::BaseItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}
