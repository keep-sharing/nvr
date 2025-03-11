#include "MyInputMethod.h"
#include <QComboBox>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QtDebug>

MyInputMethod::MyInputMethod()
{
    m_keyboard = new Keyboard(nullptr);
    m_keyboard->hide();
}

void MyInputMethod::updateHandler(int type)
{
    switch (type) {
    case Update:
        break;
    case FocusIn: {
        bool isShow = false;
        QWidget *focusWidget = QApplication::focusWidget();
        //qDebug() << QString("MyInputMethod::updateHandler, focusWidget:") << focusWidget;
        //
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(focusWidget);
        if (lineEdit) {
            isShow = true;
        }
        //
        QSpinBox *spinBox = qobject_cast<QSpinBox *>(focusWidget);
        if (spinBox) {
            isShow = true;
            if (spinBox->property("ShowKeyboard").toInt() == -12345) {
                isShow = false;
            }
        }
        //
        QComboBox *comboBox = qobject_cast<QComboBox *>(focusWidget);
        if (comboBox) {
            isShow = true;
            if (comboBox->property("ShowKeyboard").toInt() == -12345) {
                isShow = false;
            }
        }
        QTextEdit *textEdit = qobject_cast<QTextEdit *>(focusWidget);
        if (textEdit) {
            isShow = true;
            if (textEdit->property("ShowKeyboard").toInt() == -12345) {
                isShow = false;
            }
        }
        //
        if (isShow) {
            QPoint point = focusWidget->mapToGlobal(QPoint(0, 0));

            QRect screenRect = qApp->desktop()->screenGeometry();
            if (point.x() + m_keyboard->width() > screenRect.right()) {
                point.setX(screenRect.right() - m_keyboard->width());
            }
            if (point.y() + m_keyboard->height() > screenRect.bottom()) {
                point.setY(point.y() - m_keyboard->height());
            } else {
                point.setY(point.y() + focusWidget->height());
            }

            showKeyboard(point);
        }
        break;
    }
    case FocusOut: {
        hideKeyboard();
        break;
    }
    case Reset:
        break;
    case Destroyed:
        break;
    default:
        break;
    }
}

void MyInputMethod::showKeyboard(const QPoint &pos)
{
    m_keyboard->move(pos);
    m_keyboard->show();
}

void MyInputMethod::hideKeyboard()
{
    m_keyboard->hide();
}
