#ifndef AUTOLOGOUT_H
#define AUTOLOGOUT_H

#include <QObject>
#include <QTimer>

#define gAutoLogout AutoLogout::instance()

class AutoLogout : public QObject
{
    Q_OBJECT
public:
    explicit AutoLogout(QObject *parent = 0);
    ~AutoLogout();

    static AutoLogout &instance();

    int readInterval();
    void writeInterval(int sec);
    int interval() const;

    void resume();
    void refreshTimeout();
    void setInterval(int sec);

    bool isReadyLogout() const;
    void logout();

private:
    void updateTipValue();
    void clearTipValue();

signals:
    void logouted();

private slots:
    void onTimerLogout();

private:
    int m_count = 0;
    int m_interval = 0;
    QTimer *m_timer = nullptr;
    bool m_isReadyLogout = false;
};

#endif // AUTOLOGOUT_H
