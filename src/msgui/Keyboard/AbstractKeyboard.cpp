#include "AbstractKeyboard.h"

AbstractKeyboard::AbstractKeyboard(QWidget *parent)
    : QWidget(parent)
{

}

QString AbstractKeyboard::buttonText(const QString &name) const
{
    Q_UNUSED(name)

    return QString();
}
