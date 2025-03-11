#include "KeyButton.h"
#include "KeyboardData.h"
#include "MyDebug.h"
#include "screencontroller.h"
#include <QMouseEvent>
#include <QPainter>
#include <QWSEvent>
#include <QWSServer>
#include "AbstractKeyboard.h"

KeyButton::KeyButton(QWidget *parent)
    : QToolButton(parent)
{
    setAutoRepeat(true);
    setAutoRepeatDelay(500);

    connect(this, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(&gKeyboardData, SIGNAL(sigUpdateAllKeys()), this, SLOT(update()));
}

QString KeyButton::initialize(AbstractKeyboard *keyboard)
{
    m_keyboard = keyboard;
    if (!m_keyboard) {
        qMsCritical() << "error keyboard:" << keyboard;
    }

    m_name = objectName();
    m_name.remove("toolButton_");
    if (m_name.isEmpty()) {
        qMsWarning() << "error name:" << m_name;
    }

    return m_name;
}

bool KeyButton::qwsEvent(QWSEvent *event)
{
    QWSMouseEvent *mouseEvent = event->asMouse();

    if (ScreenController::instance()->refreshRate() < 60 && mouseEvent && mouseEvent->simpleData.state >= 1) {
        m_qwsPressed = !m_qwsPressed;
        if (m_qwsPressed) {
            m_pressCount++;
        }
        qDebug() << "press count: " << m_pressCount;
    }

    return false;
}

void KeyButton::mousePressEvent(QMouseEvent *event)
{
    if (ScreenController::instance()->refreshRate() < 60 && Q_UNLIKELY(--m_pressCount < 0)) {
        qDebug() << "skip mouse press due to press count < 0";
        m_pressCount = 0;
        return;
    }

    m_isPressed = true;
    QToolButton::mousePressEvent(event);

    update();
}

void KeyButton::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    QToolButton::mouseReleaseEvent(event);

    update();
}

void KeyButton::enterEvent(QEvent *event)
{
    m_isHover = true;
    QToolButton::enterEvent(event);

    update();
}

void KeyButton::leaveEvent(QEvent *event)
{
    m_isHover = false;
    QToolButton::leaveEvent(event);

    update();
}

void KeyButton::showEvent(QShowEvent *event)
{
    QToolButton::showEvent(event);
}

void KeyButton::hideEvent(QHideEvent *event)
{
    m_qwsPressed = false;
    m_pressCount = 0;

    m_isHover   = false;
    m_isPressed = false;

    QToolButton::hideEvent(event);
}

void KeyButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QColor backgroundColor;
    QColor textColor(255, 255, 255);
    QString text;

    const auto &key = gKeyboardData.key(m_name);

    //背景
    painter.setPen(Qt::NoPen);
    if (m_isHover) {
        backgroundColor = QColor(102, 182, 219);
    }
    if (m_isPressed) {
        backgroundColor = QColor(102, 182, 219);
        textColor       = QColor(0, 0, 0);
    }
    if (!m_isHover && !m_isPressed) {
        backgroundColor = QColor(80, 80, 80);
    }
    painter.setBrush(backgroundColor);
    painter.drawRect(rect());

    //文字
    if (key.isControl()) {
        textColor = QColor(205, 137, 0);
    }
    painter.setPen(textColor);
    text = m_keyboard->buttonText(m_name);
    painter.drawText(rect(), Qt::AlignCenter, text);
}

void KeyButton::onClicked()
{
    emit keyClicked(m_name);
}
