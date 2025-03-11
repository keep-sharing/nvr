#include "mylabel.h"
#include "MsLanguage.h"
#include <QFontMetrics>

MyLabel::MyLabel(QWidget *parent)
    : QLabel(parent)
{
}

void MyLabel::setTranslatableText(const QString &key, const QString &defaultValue)
{
    m_textKey = key;
    m_defaultValue = defaultValue;

    const QString &value = MsLanguage::instance()->value(m_textKey, defaultValue);
    setText(value);
}

void MyLabel::retranslate()
{
    const QString &value = MsLanguage::instance()->value(m_textKey, m_defaultValue);
    setText(value);
}

void MyLabel::setElidedText(const QString &text)
{
    m_wholeText = text;

    resetElidedText();
}

void MyLabel::clear()
{
    m_textKey.clear();
    m_defaultValue.clear();
    m_wholeText.clear();
    QLabel::clear();
}

void MyLabel::resizeEvent(QResizeEvent *event)
{
    if (!m_wholeText.isEmpty()) {
        resetElidedText();
    }

    QLabel::resizeEvent(event);
}

void MyLabel::resetElidedText()
{
    QFontMetrics fm(font());
    const QString &elidedText = fm.elidedText(m_wholeText, Qt::ElideRight, width());
    setText(elidedText);
}
