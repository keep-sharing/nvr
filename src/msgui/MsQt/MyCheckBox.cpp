#include "MyCheckBox.h"
#include <QEvent>
#include <QStyle>
#include <QtDebug>

MyCheckBox::MyCheckBox(QWidget *parent)
    : QCheckBox(parent)
{
    installEventFilter(this);
}

void MyCheckBox::setCheckState(Qt::CheckState state)
{
    QCheckBox::setCheckState(state);

    emit checkStateSet(state);
}

void MyCheckBox::setChecked(bool checked)
{
    QCheckBox::setChecked(checked);

    //
    Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    emit checkStateSet(state);
}

void MyCheckBox::reset()
{
    Qt::CheckState state = checkState();
    emit checkStateSet(state);
}

void MyCheckBox::updateStyle()
{
    style()->unpolish(this);
    style()->polish(this);
}

bool MyCheckBox::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Wheel:
        e->ignore();
    default:
        break;
    }
    return QCheckBox::event(e);
}

bool MyCheckBox::eventFilter(QObject *obj, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseButtonPress: {
        return true;
    }
    case QEvent::MouseButtonRelease: {
        if (isEnabled()) {
            Qt::CheckState state = checkState();
            bool checked = false;
            if (state != Qt::Checked) {
                checked = true;
            }
            setChecked(checked);
            emit clicked(checked);
        }
        return true;
    }
    default:
        break;
    }
    return QCheckBox::eventFilter(obj, e);
}
