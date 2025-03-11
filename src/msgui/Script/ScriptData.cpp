#include "ScriptData.h"
#include "MyDebug.h"
#include "screencontroller.h"
#include <QElapsedTimer>
#include <QTimer>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

ScriptData::ScriptData(QObject *parent)
    : QObject(parent)
{
    moveToThread(&m_thread);
    m_thread.start();

    char buf[256];
    memset(buf, 0, sizeof(buf));
    for (int i = 0; i < 6; ++i) {
        QString mouse = QString("/dev/input/event%1").arg(i);
        int fd = open(mouse.toLocal8Bit().constData(), O_RDONLY, 0);
        if (fd >= 0) {
            ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
            close(fd);
            qMsDebug() << mouse << "EVIOCGNAME:" << buf;
            if (QString(buf).contains("mouse", Qt::CaseInsensitive)) {
                if (m_fd >= 0) {
                    close(m_fd);
                }
                m_fd = open(mouse.toLocal8Bit().constData(), O_RDWR);
            }
        }
    }
}

ScriptData::~ScriptData()
{
    if (m_fd > 0) {
        close(m_fd);
        m_fd = -1;
    }
}

ScriptData &ScriptData::instance()
{
    static ScriptData self;
    return self;
}

void ScriptData::clearCommand()
{
    QMutexLocker locker(&m_mutex);

    m_cmdList.clear();
    m_currentIndex = 0;
}

void ScriptData::appendCommand(ScriptCommand *cmd)
{
    QMutexLocker locker(&m_mutex);

    cmd->setIndex(m_cmdList.size());
    m_cmdList.append(*cmd);
}

void ScriptData::startScript()
{
    QMutexLocker locker(&m_mutex);

    m_isRunning = true;
    QMetaObject::invokeMethod(this, "onStartScript", Qt::QueuedConnection);
}

void ScriptData::stopScript()
{
    QMutexLocker locker(&m_mutex);

    m_isRunning = false;
    QMetaObject::invokeMethod(this, "onStopScript", Qt::QueuedConnection);
}

void ScriptData::mouse_move(int fd, int rel_x, int rel_y)
{
    struct input_event event;
    gettimeofday(&event.time, 0);
    //x轴坐标的相对位移
    event.type = EV_REL;
    event.value = rel_x;
    event.code = REL_X;
    write(fd, &event, sizeof(event));
    //y轴坐标的相对位移
    event.type = EV_REL;
    event.value = rel_y;
    event.code = REL_Y;
    write(fd, &event, sizeof(event));
    //同步
    event.type = EV_SYN;
    event.value = 0;
    event.code = SYN_REPORT;
    write(fd, &event, sizeof(event));
}

void ScriptData::mouse_move(int fd, const QPoint &p, int delay)
{
    if (delay > 0) {
        usleep(1000 * delay);
    }

    qreal accel = ScreenController::instance()->mouseRealAccel();

    //只能相对坐标移动，所以先移动到左上角
    mouse_move(fd, -99999, -99999);
    usleep(10000);
    mouse_move(fd, p.x() / accel, p.y() / accel);
}

void ScriptData::mouse_press(int fd, int button, int delay)
{
    if (delay > 0) {
        usleep(1000 * delay);
    }

    //
    struct input_event event;
    gettimeofday(&event.time, 0);
    event.code = button;
    event.type = EV_KEY;
    //按下
    event.value = 1;
    write(fd, &event, sizeof(event));

    struct input_event event_syn;
    event_syn.type = EV_SYN;
    event_syn.value = 0;
    event_syn.code = SYN_REPORT;
    write(fd, &event_syn, sizeof(event_syn));
}

void ScriptData::mouse_release(int fd, int button, int delay)
{
    if (delay > 0) {
        usleep(1000 * delay);
    }

    //
    struct input_event event;
    gettimeofday(&event.time, 0);
    event.code = button;
    event.type = EV_KEY;
    //弹起
    event.value = 0;
    write(fd, &event, sizeof(event));

    struct input_event event_syn;
    event_syn.type = EV_SYN;
    event_syn.value = 0;
    event_syn.code = SYN_REPORT;
    write(fd, &event_syn, sizeof(event_syn));
}

void ScriptData::mouse_click(int fd, int key)
{
    struct input_event event_syn;
    event_syn.type = EV_SYN;
    event_syn.value = 0;
    event_syn.code = SYN_REPORT;

    struct input_event event;
    gettimeofday(&event.time, 0);
    event.code = key;
    event.type = EV_KEY;
    //按下
    event.value = 1;
    write(fd, &event, sizeof(event));
    write(fd, &event_syn, sizeof(event_syn));
    //
    usleep(10000);
    //弹起
    event.value = 0;
    write(fd, &event, sizeof(event));
    write(fd, &event_syn, sizeof(event_syn));
}

bool ScriptData::isRunning()
{
    QMutexLocker locker(&m_mutex);
    return m_isRunning;
}

void ScriptData::onStartScript()
{
    if (m_fd < 0) {
        qMsWarning() << "invalid fd:" << m_fd;
        return;
    }

    if (m_cmdList.isEmpty()) {
        qMsWarning() << "script: empty.";
        return;
    }

    //
    if (!isRunning()) {
        return;
    }

    //
    if (m_currentIndex >= m_cmdList.size()) {
        emit once();
        m_currentIndex = 0;
        //执行完一轮后延时1秒开始下一轮
        QTimer::singleShot(1000, this, SLOT(onStartScript()));
        return;
    }
    qMsDebug() << m_currentIndex + 1;
    emit indexChanged(m_currentIndex);
    m_currentCmd = m_cmdList.at(m_currentIndex);
    m_currentIndex++;

    //执行
    const auto &infos = m_currentCmd.infos();
    int button;
    switch (infos.first().button) {
    case Qt::LeftButton:
        button = BTN_LEFT;
        break;
    case Qt::RightButton:
        button = BTN_RIGHT;
        break;
    default:
        return;
    }
    //
    for (int i = 0; i < infos.size(); ++i) {
        const auto &info = infos.at(i);
        int delay = info.delay;
        if (i == 0) {
            mouse_move(m_fd, info.point, delay);
            delay = 1000;
        }
        switch (info.type) {
        case QEvent::MouseButtonPress:
            mouse_press(m_fd, button, delay);
            break;
        case QEvent::MouseButtonRelease:
            mouse_release(m_fd, button, delay);
            break;
        case QEvent::MouseButtonDblClick:
            mouse_press(m_fd, button, delay);
            break;
        default:
            break;
        }
    }

    //
    QTimer::singleShot(0, this, SLOT(onStartScript()));
}

void ScriptData::onStopScript()
{
    qMsDebug();
}
