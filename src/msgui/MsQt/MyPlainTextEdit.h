#ifndef MYPLAINTEXTEDIT_H
#define MYPLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <QElapsedTimer>
#include <QTimer>

class MyPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit MyPlainTextEdit(QWidget *parent = nullptr);

    void appendMessage(QColor color, const QString &text);

private:
    bool isScrollbarAtBottom() const;

private slots:
    void scrollToBottom();

private:
    QTextCursor m_textCursor;

    QTimer m_scrollTimer;
    QElapsedTimer m_lastMessage;
};

#endif // MYPLAINTEXTEDIT_H
