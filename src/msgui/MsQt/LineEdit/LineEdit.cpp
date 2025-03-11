#include "LineEdit.h"
#include "MyDebug.h"
#include "MyLineEditTip.h"
#include <QEvent>
#include <QStyle>
#include <QTimer>

LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);

    m_timerTip = new QTimer(this);
    connect(m_timerTip, SIGNAL(timeout()), this, SLOT(onTipTimeout()));

    m_invalidTip = new MyLineEditTip(this);
}

bool LineEdit::isValid() const
{
    return m_valid;
}

void LineEdit::setValid(bool newValid)
{
    if (m_valid == newValid) {
        return;
    }
    m_valid = newValid;
    //
    style()->polish(this);
    //
    if (m_valid) {
        hideWarningTip();
    } else {
        showWarningTip();
    }
    emit validChanged();
}

void LineEdit::clearWarning()
{
    setValid(true);
}

bool LineEdit::checkValid()
{
    setValid(check());
    return isValid();
}

void LineEdit::showCustomTip()
{
}

void LineEdit::showEvent(QShowEvent *event)
{
    QLineEdit::showEvent(event);
}

void LineEdit::hideEvent(QHideEvent *event)
{
    setValid(true);
    QLineEdit::hideEvent(event);
}

void LineEdit::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::EnabledChange) {
        if (isEnabled()) {
            if (!isValid()) {
                QMetaObject::invokeMethod(this, "showWarningTip", Qt::QueuedConnection);
            }
        } else {
            QMetaObject::invokeMethod(this, "hideWarningTip", Qt::QueuedConnection);
            if (!m_invalidTip) {
                m_invalidTip = new MyLineEditTip(this);
            }
            m_invalidTip->setEnabled(true);
            m_invalidTip->update();
        }
    }
    QLineEdit::changeEvent(event);
}

int LineEdit::tipHeight() const
{
    return 26;
}

void LineEdit::showWarningTip()
{
    QPoint p = mapToGlobal(QPoint(0, 0));
    QRect rc(p.x(), p.y() + height() - 1, width(), tipHeight());
    m_invalidTip->setGeometry(rc);
    m_invalidTip->setText(tipString());
    m_invalidTip->show();
    m_timerTip->start(5000);
}

void LineEdit::hideWarningTip()
{
    m_invalidTip->hide();
}

void LineEdit::onTipTimeout()
{
    clearWarning();
}
