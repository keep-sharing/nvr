#include "ToolButtonFace.h"

ToolButtonFace::ToolButtonFace(QWidget *parent)
    : QToolButton(parent)
{

}

void ToolButtonFace::setElidedText(const QString &text)
{
    m_wholeText = text;
    QFontMetrics fm(font());
    const QString &elidedText = fm.elidedText(text, Qt::ElideRight, width());
    setText(elidedText);
}

QString ToolButtonFace::wholeText() const
{
    return m_wholeText;
}

void ToolButtonFace::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void ToolButtonFace::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}
