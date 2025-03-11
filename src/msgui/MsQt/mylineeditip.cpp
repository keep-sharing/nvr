#include "mylineeditip.h"

MyLineEditIP::MyLineEditIP(QWidget *parent) :
    QLineEdit(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);

    setInputMask("000.000.000.000");
}

QString MyLineEditIP::text()
{
    QString str = QLineEdit::text();
    if (str == QString("..."))
    {
        str.clear();
    }
    return str;
}
