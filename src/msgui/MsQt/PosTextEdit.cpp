#include "PosTextEdit.h"
#include "MyDebug.h"
#include "PosData.h"
#include <QPainter>
#include <QScrollBar>
#include <QTimer>

PosTextEdit::PosTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setStyleSheet("border:0px;background:gransparent;");
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);

    setContextMenuPolicy(Qt::NoContextMenu);
    setFrameShape(QPlainTextEdit::NoFrame);
    setReadOnly(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    document()->setDocumentMargin(0);
    document()->setMaximumBlockCount(100);

    m_textCursor = textCursor();

    m_timeout = new QTimer(this);
    connect(m_timeout, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timeout->setInterval(100);
}

void PosTextEdit::appendText(const PosData &data)
{
    clear();

    QTextCharFormat format;
    format.setForeground(data.fontColor());
    QFont f = font();
    f.setPointSize(data.fontSize());
    format.setFont(f);

    m_textCursor.movePosition(QTextCursor::End);
    m_textCursor.insertText(data.text().trimmed(), format);

    scrollToBottom();
    m_timeoutValue = 0;
    m_timeoutInterval = data.showTime() * 10;
    if (!m_timeout->isActive()) {
        m_timeout->start();
    }
}

void PosTextEdit::setPaused(bool pause)
{
    if (pause) {
        m_timeout->stop();
    } else {
        m_timeout->start();
    }
}

void PosTextEdit::setPosGeometry(const PosData &data, const QRect &videoRect)
{
    m_posArea = data.posArea();
    QRect rc = data.geometry(videoRect).toRect();
    setGeometry(rc);
}

void PosTextEdit::resetPosGeometry(const QRect &videoRect)
{
    QRect rc = PosData::getPosGeometry(m_posArea, videoRect).toRect();
    setGeometry(rc);
}

void PosTextEdit::paintEvent(QPaintEvent *e)
{
    QPlainTextEdit::paintEvent(e);

#if 0
    QPainter painter(viewport());
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 0, 0, 40));
    painter.drawRect(rect());
#endif
}

void PosTextEdit::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    // QPlainTextEdit destroys the first calls value in case of multiline
    // text, so make sure that the scroll bar actually gets the value set.
    // Is a noop if the first call succeeded.
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void PosTextEdit::onTimeout()
{
    m_timeoutValue++;
    if (m_timeoutValue > m_timeoutInterval) {
        clear();
        m_timeoutValue = 0;
        m_timeout->stop();
    }
}
