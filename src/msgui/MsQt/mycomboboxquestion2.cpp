#include "mycomboboxquestion2.h"
#include "mylineedit.h"
#include "MyLineEditTip.h"
#include "MsLanguage.h"
#include <QPainter>
#include <QStyle>
#include <QTimer>

const int TipHeight = 26;

MyComboBoxQuestion2::MyComboBoxQuestion2(QWidget *parent) :
    MyComboBoxQuestion(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);
    //
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->setInterval(5000);
}

bool MyComboBoxQuestion2::isValid() const
{
    return m_valid;
}

void MyComboBoxQuestion2::setValid(bool newValid)
{
    if (m_valid == newValid) {
        if (!m_valid) {
            showWarningTip();
        }
        return;
    }
    m_valid = newValid;
    style()->polish(this);
    update();

    if (m_valid) {
        hideWarningTip();
    } else {
        showWarningTip();
    }

    emit validChanged();
}

bool MyComboBoxQuestion2::checkValid()
{
    if (!isVisible() ) {
        return true;
    }
    onEditFinished();
    return isValid();
}

void MyComboBoxQuestion2::setTipString(const QString &str)
{
    if (!m_invalidTip) {
        m_invalidTip = new MyLineEditTip(this);
    }
    m_invalidTip->setText(str);
}

QString MyComboBoxQuestion2::lineEditStyleSheet() const
{
    QString text = QString("QLineEdit\
                                {\
                                min-height: 28px;\
                                background: transparent;\
                                color: #4A4A4A;\
                                padding-left: 0px;\
                                padding-right: 0px;\
                                border: 0px solid #b9b9b9;\
                                }");
                           return text;
}

void MyComboBoxQuestion2::hideEvent(QHideEvent *event)
{
    setValid(true);
    QWidget::hideEvent(event);
}

void MyComboBoxQuestion2::showWarningTip()
{
    if (!isEnabled()) {
        return;
    }
    if (!isVisible()) {
        return;
    }

    if (!m_invalidTip) {
        m_invalidTip = new MyLineEditTip(this);
    }
    QPoint p = mapToGlobal(QPoint(0, 0));
    QRect rc(p.x(), p.y() + height() - 1, width(), TipHeight);
    m_invalidTip->setGeometry(rc);
    m_invalidTip->show();
    m_timer->start();
}

void MyComboBoxQuestion2::hideWarningTip()
{
    if (m_invalidTip) {
        m_invalidTip->hide();
    }
}

void MyComboBoxQuestion2::onEditFinished()
{
    int index = currentIndex();
    if (index != 12) {
        setValid(true);
        return;
    }
    QLineEdit *edit = lineEdit();
    if (edit) {
        const QString &str = edit->text();
        do {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            //valid
            setValid(true);
            return;
        } while (0);
    } else {
        setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
    }
    //invalid
    setValid(false);
}

void MyComboBoxQuestion2::onTimeout()
{
    setValid(true);
}
