#ifndef TESTHARDWAREDATA_H
#define TESTHARDWAREDATA_H

#include <QObject>
#include <QThread>
#include <QColor>
#include <QMutex>
#include "msstd.h"
#include "mshw/mshw.h"

class TestHardwareData : public QObject
{
    Q_OBJECT
public:
    explicit TestHardwareData(QObject *parent = nullptr);
    ~TestHardwareData() override;

    static TestHardwareData *instance();

    void stopThread();

    void showWizardMessage(const QString &text);
    void showMessage(QColor color, const QString &text);

    void setVideoParams(const TestVideo &params);

    bool isItemRunning();
    void setItemRunning(bool newItemRunning);

signals:
    void wizardMessage(const QString &text);
    void message(QColor color, const QString &text);

public slots:
    void testItemStart(const QString &name);
    void testItemStop();

private:
    static TestHardwareData *s_self;
    QThread m_thread;
    QMutex m_mutex;

    bool m_isItemRunning = false;

    TestVideo *m_videoParams = nullptr;
};

#endif // TESTHARDWAREDATA_H
