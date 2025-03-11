#include "MyLineEditTip.h"
#include <QPainter>
#include <QToolButton>
#include <QDebug>
MyLineEditTip::MyLineEditTip(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    m_toolButton = new QToolButton(this);
    m_toolButton->setIconSize(QSize(14, 14));
    m_toolButton->setIcon(QIcon(":/common/common/warning_red.png"));
    m_toolButton->setStyleSheet("border: 0px; padding: 0px;background-color: rgb(255, 255, 0, 0)");
}

MyLineEditTip::~MyLineEditTip()
{
}

void MyLineEditTip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);

    QRect rc = rect();
    rc.adjust(0, 0, -1, -1);
    painter.setPen(QPen(m_borderColor, 1));
    painter.setBrush(m_backgroundColor);
    painter.drawRect(rc);

    QRect textRect = rc;
    textRect.moveLeft(30);
    //根据提示语长度设定字号
    int textWidth = QFontMetrics(painter.font()).width(m_text);
    int rcWidth = rc.width() - QSize(26, 14).width();
    QFont f = font();
    if (textWidth > rcWidth) {
        f.setPointSize(11);
        painter.setFont(f);
    }
    if (m_fontSize > 0) {
        f.setPointSize(m_fontSize);
        painter.setFont(f);
    }
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, m_text);
}

const QString &MyLineEditTip::text() const
{
    return m_text;
}

void MyLineEditTip::setText(const QString &newText)
{
    m_text = newText;
    update();
}

void MyLineEditTip::setTextSize(int size)
{
    m_fontSize = size;
}

void MyLineEditTip::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void MyLineEditTip::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    m_toolButton->move(10, (height() - m_toolButton->height()) / 2 + 1);
}
