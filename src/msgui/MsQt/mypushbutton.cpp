#include "mypushbutton.h"
#include <QKeyEvent>
#include "MsLanguage.h"

MyPushButton::MyPushButton(QWidget *parent) :
    QPushButton(parent)
{

}

MyPushButton::MyPushButton(const QString &text, QWidget *parent) :
    QPushButton(text, parent)
{

}

void MyPushButton::setTranslatableText(const QString &key, const QString &defaultValue)
{
    m_textKey = key;
    m_defaultValue = defaultValue;

    const QString &value = MsLanguage::instance()->value(m_textKey, defaultValue);
    setText(value);
}

void MyPushButton::retranslate()
{
    const QString &value = MsLanguage::instance()->value(m_textKey, m_defaultValue);
    setText(value);
}

void MyPushButton::clearUnderMouse()
{
    setAttribute(Qt::WA_UnderMouse, false);
}

void MyPushButton::clearHover()
{
    clearUnderMouse();
    clearFocus();
}

void MyPushButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
}

void MyPushButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    clearUnderMouse();
}

void MyPushButton::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        event->accept();
        emit clicked();
        break;
    default:
        break;
    }
}
