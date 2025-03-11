#include "MyPlainTextEdit.h"
#include <QScrollBar>

MyPlainTextEdit::MyPlainTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    m_textCursor = textCursor();

    m_scrollTimer.setInterval(10);
    m_scrollTimer.setSingleShot(true);
    connect(&m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollToBottom()));

    //m_lastMessage.start();
}

void MyPlainTextEdit::appendMessage(QColor color, const QString &text)
{
#if 0
    const bool atBottom = isScrollbarAtBottom() || m_scrollTimer.isActive();

    //
    if (!m_textCursor.atEnd()) {
        m_textCursor.movePosition(QTextCursor::End);
    }

    QTextCharFormat format;
    format.setForeground(color);

    m_textCursor.insertText(text + "\n", format);

    //
    if (atBottom) {
        if (m_lastMessage.elapsed() < 5) {
            m_scrollTimer.start();
        } else {
            m_scrollTimer.stop();
            scrollToBottom();
        }
    }
    m_lastMessage.start();
#else
    QTextCharFormat format;
    format.setForeground(color);
    m_textCursor.insertText(text + "\n", format);

    scrollToBottom();
#endif
}

bool MyPlainTextEdit::isScrollbarAtBottom() const
{
    return verticalScrollBar()->value() == verticalScrollBar()->maximum();
}

void MyPlainTextEdit::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    // QPlainTextEdit destroys the first calls value in case of multiline
    // text, so make sure that the scroll bar actually gets the value set.
    // Is a noop if the first call succeeded.
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}
