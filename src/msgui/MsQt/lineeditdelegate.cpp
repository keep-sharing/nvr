#include "lineeditdelegate.h"
#include "MyDebug.h"
#include <QKeyEvent>

LineEditDelegate::LineEditDelegate(QWidget *parent)
    : QLineEdit(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);
    connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}

LineEditDelegate::~LineEditDelegate()
{

}

void LineEditDelegate::onEditingFinished()
{
    setNewText(text());
    emit sigEditintFinished(m_row, m_column, m_oldText, m_newText);
}

QString LineEditDelegate::newText() const
{
    return m_newText;
}

void LineEditDelegate::setNewText(const QString &newText)
{
    m_newText = newText;
}

void LineEditDelegate::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return;
    default:
        break;
    }

    return QLineEdit::keyPressEvent(event);
}

QString LineEditDelegate::oldText() const
{
    return m_oldText;
}

void LineEditDelegate::setOldText(const QString &oldText)
{
    m_oldText = oldText;
}

int LineEditDelegate::column() const
{
    return m_column;
}

void LineEditDelegate::setColumn(int column)
{
    m_column = column;
}

int LineEditDelegate::row() const
{
    return m_row;
}

void LineEditDelegate::setRow(int row)
{
    m_row = row;
}
