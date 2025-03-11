#include "mylineeditmac.h"

MyLineEditMac::MyLineEditMac(QWidget *parent) :
    QLineEdit(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);

    setInputMask(">HH:HH:HH:HH:HH:HH");
}

QString MyLineEditMac::text()
{
    QString str = QLineEdit::text();
    if (str == QString(":::::"))
    {
        str.clear();
    }
    return str;
}
