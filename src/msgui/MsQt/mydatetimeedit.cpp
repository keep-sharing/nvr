#include "mydatetimeedit.h"

MyDateTimeEdit::MyDateTimeEdit(QWidget *parent) :
    QDateTimeEdit(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);
    setDisplayFormat("yyyy-MM-dd HH:mm:ss");
}
