#ifndef SCRIPT_H
#define SCRIPT_H

#include "BaseShadowDialog.h"
#include <QDateTime>
#include <QElapsedTimer>
#include "ScriptCommand.h"

namespace Ui {
class Script;
}

class Script : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit Script(QWidget *parent = nullptr);
    ~Script();

    static Script *instance();

    bool isPlaying() const;
    bool isRecording() const;
    void appendCommond(QObject *receiver, QEvent *e);

    void scriptRun();
    void scriptStop();

private:
    QRect globalRect() const;

    void startPlay();
    void stopPlay();

private slots:
    void onRecordTimer();
    //运行时间
    void onRunTimer();

    //
    void onScriptIndexChanged(int index);
    //执行完一轮
    void onScriptRunOnce();

    void on_toolButtonRecord_clicked();
    void on_toolButtonPlay_clicked();

private:
    static Script *self;

    Ui::Script *ui;

    bool m_isRecording = false;
    QElapsedTimer m_timeline;
    QElapsedTimer m_delayTimer;
    QTimer *m_recordTimer = nullptr;

    QTimer *m_runTimer = nullptr;
    QDateTime m_timeBegin;

    int m_count = 0;

    bool m_isPlaying = false;

    //收到mouse release时生成一条指令，之前的先存起来
    ScriptCommand m_tempCmd;
};

#endif // SCRIPT_H
