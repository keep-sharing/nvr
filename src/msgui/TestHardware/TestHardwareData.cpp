#include "TestHardwareData.h"
#include <QMutexLocker>

TestHardwareData *TestHardwareData::s_self = nullptr;

void callback_result(const char *mod, const char *log, LogLevel flag)
{
    Q_UNUSED(mod)
    Q_UNUSED(flag)

    if (TestHardwareData::instance()) {
        if (flag == LOG_NOTE) {
            TestHardwareData::instance()->showWizardMessage(QString("%1").arg(log));
        } else {
            QColor color = Qt::black;
            switch (flag) {
            case LOG_NONE:
                break;
            case LOG_INFO:
                break;
            case LOG_ERR:
                color = Qt::darkRed;
                break;
            case LOG_SUCCESS:
                color = Qt::darkGreen;
                break;
            default:
                break;
            }
            TestHardwareData::instance()->showMessage(color, QString("%1").arg(log));
        }
    }
}

TestHardwareData::TestHardwareData(QObject *parent)
    : QObject(parent)
{
    s_self = this;
    moveToThread(&m_thread);
    m_thread.start();
}

TestHardwareData::~TestHardwareData()
{
    if (m_videoParams) {
        delete m_videoParams;
    }
    s_self = nullptr;
}

TestHardwareData *TestHardwareData::instance()
{
    return s_self;
}

void TestHardwareData::stopThread()
{
    m_thread.quit();
    m_thread.wait();
}

void TestHardwareData::showWizardMessage(const QString &text)
{
    emit wizardMessage(text);
}

void TestHardwareData::showMessage(QColor color, const QString &text)
{
    emit message(color, text);
}

void TestHardwareData::setVideoParams(const TestVideo &params)
{
    if (!m_videoParams) {
        m_videoParams = new TestVideo;
    }
    memcpy(m_videoParams, &params, sizeof(TestVideo));
}

void TestHardwareData::testItemStart(const QString &name)
{
    setItemRunning(true);
    if (name == QString(TEST_NAME_VIDEO) && m_videoParams) {
        ms_hw_test_start(name.toLocal8Bit().constData(), callback_result, m_videoParams);
    } else {
        ms_hw_test_start(name.toLocal8Bit().constData(), callback_result, NULL);
    }
    setItemRunning(false);
}

void TestHardwareData::testItemStop()
{
    ms_hw_test_stop();
}

bool TestHardwareData::isItemRunning()
{
    QMutexLocker locker(&m_mutex);
    return m_isItemRunning;
}

void TestHardwareData::setItemRunning(bool newItemRunning)
{
    QMutexLocker locker(&m_mutex);
    m_isItemRunning = newItemRunning;
}
