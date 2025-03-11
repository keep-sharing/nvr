#include "lineeditip.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QRegExpValidator>
#include <QEvent>
#include <QKeyEvent>
#include "MyInputMethod.h"

LineEditIP::LineEditIP(QWidget *parent) :
    QLineEdit(parent)
{
    QRegExp rx("(2[0-5]{2}|2[0-4][0-9]|1?[0-9]{1,2})");
    QHBoxLayout *pHBox = new QHBoxLayout(this);
    pHBox->setSpacing(10);
    pHBox->setContentsMargins(1, 1, 1, 1);
    for (int i = 0; i < 4; i++)
    {
        QLineEdit *edit = new QLineEdit(this);
        connect(edit, SIGNAL(textEdited(QString)), this, SLOT(onTextEdit(QString)));
        edit->setStyleSheet(
                    "QLineEdit\
                    {\
                        height: 28px;\
                        background: transparent;\
                        padding-left: 0px;\
                        padding-right: 0px;\
                        border: 0px;\
                    }\
                    QLineEdit:hover:enabled\
                    {\
                        border: 0px;\
                    }\
                    QLineEdit:disabled\
                    {\
                        background: transparent;\
                    }\
                    QLineEdit:focus\
                    {\
                        border: 0px;\
                    }"
                    );
        edit->setFrame(false);
        edit->setContextMenuPolicy(Qt::NoContextMenu);
        edit->setMaxLength(3);
        edit->setAlignment(Qt::AlignCenter);
        edit->installEventFilter(this);
        edit->setValidator(new QRegExpValidator(rx, this));
        edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        edit->setMaximumWidth(40);
        pHBox->addWidget(edit);
        m_lineEditList.append(edit);
    }
    pHBox->addStretch();

    setReadOnly(true);
}


void LineEditIP::setText(const QString &strIP)
{
    if (!isTextValid(strIP))
    {
        //QMessageBox::warning(this, "Attention", "Your IP Address is Invalid!", QMessageBox::StandardButton::Ok);
        return;
    }
    else
    {
        int i = 0;
        QStringList ipList = strIP.split(".");
        for (const QString &ip : ipList)
        {
            m_lineEditList.at(i)->setText(ip);
            i++;
        }
    }
}

QString LineEditIP::text() const
{
    QString strIP;
    for (int i = 0; i < 4; i++)
    {
        QString str = m_lineEditList.at(i)->text();
        if (str.isEmpty())
        {
            strIP.clear();
            break;
        }
        strIP.append(str);
        if (i < 3)
        {
            strIP.append(".");
        }
    }
    return strIP;
}

void LineEditIP::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);

    QPainter painter(this);
    QBrush brush;
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    brush.setColor(Qt::black);
    painter.setBrush(brush);

    int width = 0;
    for (int i = 0; i < 3; i++)
    {
        width += m_lineEditList.at(i)->width() + (i == 0 ? 3 : 10);//布局的间隔
        painter.drawEllipse(width, height() / 2 - 2, 2, 2);
    }
}

bool LineEditIP::eventFilter(QObject *obj, QEvent *ev)
{
    if (children().contains(obj) && QEvent::KeyPress == ev->type())
    {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(ev);
        QLineEdit *pEdit = qobject_cast<QLineEdit *>(obj);
        switch (keyEvent->key())
        {
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        {
            QString strText = pEdit->text();
            if (pEdit->selectedText().length())
            {
                pEdit->text().replace(pEdit->selectedText(), QChar(keyEvent->key()));
            }
            else if (strText.length() == 3 || (strText.length() < 3 && strText.toInt() * 10 > 255))
            {
                int index = getIndex(pEdit);
                if (index != -1 && index != 3)
                {
                    m_lineEditList.at(index + 1)->setFocus();
                    m_lineEditList.at(index + 1)->selectAll();
                }
            }
            else if (strText.length() == 2 && strText.toInt() * 10 < 255)
            {
                if (Qt::Key_0 == keyEvent->key() && strText.toInt())
                {
                    pEdit->setText(strText.insert(pEdit->cursorPosition(), QChar(Qt::Key_0)));
                }
                int index = getIndex(pEdit);
                if (index != -1 && index != 3)
                {
                    m_lineEditList.at(index + 1)->setFocus();
                    m_lineEditList.at(index + 1)->selectAll();
                }
            }
            return QLineEdit::eventFilter(obj, ev);
        }
        case Qt::Key_Backspace:
        {
            QString strText = pEdit->text();
            if (strText.isEmpty() || !pEdit->cursorPosition())
            {
                int index = getIndex(pEdit);
                if (index != -1 && index != 0)
                {
                    m_lineEditList.at(index - 1)->setFocus();
                    int length = m_lineEditList.at(index - 1)->text().length();
                    m_lineEditList.at(index - 1)->setCursorPosition(length ? length : 0);
                }
            }
            return QLineEdit::eventFilter(obj, ev);
        }
        case Qt::Key_Left:
        {
            if (!pEdit->cursorPosition())
            {
                int index = getIndex(pEdit);
                if (index != -1 && index != 0)
                {
                    m_lineEditList.at(index - 1)->setFocus();
                    int length = m_lineEditList.at(index - 1)->text().length();
                    m_lineEditList.at(index - 1)->setCursorPosition(length ? length : 0);
                }
            }
            return QLineEdit::eventFilter(obj, ev);
        }
        case Qt::Key_Right:
        case Qt::Key_Period:
        {
            if (pEdit->cursorPosition() == pEdit->text().length())
            {
                int index = getIndex(pEdit);
                if (index != -1 && index != 3)
                {
                    m_lineEditList.at(index + 1)->setFocus();
                    m_lineEditList.at(index + 1)->setCursorPosition(0);
                }
            }
            return QLineEdit::eventFilter(obj, ev);
        }
        default:
            break;
        }
    }
    return false;
}

void LineEditIP::mousePressEvent(QMouseEvent *event)
{
    QLineEdit *edit = m_lineEditList.first();
    if (edit)
    {
        edit->setFocus();
        edit->selectAll();
    }
    QLineEdit::mousePressEvent(event);
}

int LineEditIP::getIndex(QLineEdit *pEdit)
{
    int index = -1;
    for (int i = 0; i < 4; i++)
    {
        if (pEdit == m_lineEditList.at(i))
        {
            index = i;
            break;
        }
    }
    return index;
}

bool LineEditIP::isTextValid(const QString &strIP)
{
    QRegExp rx2("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    if (!rx2.exactMatch(strIP))
    {
        return false;
    }
    return true;
}

void LineEditIP::onTextEdit(const QString &str)
{
    Q_UNUSED(str)

    QString strIP = text();
    emit ipChanged(strIP);
}
