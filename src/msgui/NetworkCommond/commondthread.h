#ifndef COMMONDTHREAD_H
#define COMMONDTHREAD_H

#include <QThread>

enum KeyEventType {
    KeyPress,
    KeyRelease,
    KeyClick
};

enum MouseEventType {
    MouseLeftButtonClick,
    MouseRightButtonClick
};

class CommondThread : public QObject {
    Q_OBJECT

public:
    explicit CommondThread(QObject *parent = nullptr);

    void stopThread();
    void sendKeyEvent(int key, KeyEventType type);

    void sendMouseEvent(MouseEventType type);
    void sendMouseMoveEvent(int x, int y);

private:
    void simulate_key(int fd, int kval, int value);
    void mouse_move(int fd, int rel_x, int rel_y);
    void mouse_click(int fd, int key);

signals:
    void sig_sendMouseEvent(MouseEventType type);
    void sig_sendMouseMoveEvent(int x, int y);

private slots:
    void onKeyEvent(int key, KeyEventType type);
    void onSendMouseEvent(MouseEventType type);
    void onSendMouseMoveEvent(int x, int y);

private:
    QThread m_thread;
};

#endif // COMMONDTHREAD_H
