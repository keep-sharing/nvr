#include "MyTextEdit.h"

MyTextEdit::MyTextEdit(QWidget *parent)
    : QTextEdit (parent)
{
}

void MyTextEdit::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    emit editingFinished();
}
