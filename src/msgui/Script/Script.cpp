#include "Script.h"
#include "ui_Script.h"
#include "MessageBox.h"
#include "MyDebug.h"
#include "ScriptData.h"
#include <QFile>
#include <QMouseEvent>
#include <QTimer>

Script *Script::self = nullptr;

Script::Script(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::Script)
{
    self = this;
    ui->setupUi(this);
    setWindowModality(Qt::NonModal);

    setTitleWidget(ui->widget_title);

    //
    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    }
    file.close();

    //通过延时确认是单击还是双击
    m_recordTimer = new QTimer(this);
    m_recordTimer->setSingleShot(true);
    m_recordTimer->setInterval(300);
    connect(m_recordTimer, SIGNAL(timeout()), this, SLOT(onRecordTimer()));

    m_runTimer = new QTimer(this);
    m_runTimer->setInterval(1000);
    connect(m_runTimer, SIGNAL(timeout()), this, SLOT(onRunTimer()));

    connect(&gScriptData, SIGNAL(indexChanged(int)), this, SLOT(onScriptIndexChanged(int)));
    connect(&gScriptData, SIGNAL(once()), this, SLOT(onScriptRunOnce()));
}

Script::~Script()
{
    delete ui;
    self = nullptr;
}

Script *Script::instance()
{
    return self;
}

bool Script::isPlaying() const
{
    return m_isPlaying;
}

bool Script::isRecording() const
{
    return m_isRecording;
}

void Script::appendCommond(QObject *receiver, QEvent *e)
{
    Q_UNUSED(receiver)

    if (isPlaying()) {
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            if (m_isPlaying && mouseEvent->button() == Qt::MidButton) {
                stopPlay();
                return;
            }
        }
    }

    if (!isRecording()) {
        return;
    }

    switch (e->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
        if (globalRect().contains(mouseEvent->globalPos())) {
            return;
        }
        m_tempCmd.append(e->type(), mouseEvent->button(), mouseEvent->globalPos(), m_delayTimer.restart());
        if (e->type() == QEvent::MouseButtonRelease) {
            m_recordTimer->start();
        }
        break;
    }
    default:
        return;
    }
}

void Script::scriptRun()
{
    if (!m_isPlaying) {
        startPlay();
    }
}

void Script::scriptStop()
{
    if (m_isPlaying) {
        stopPlay();
    }
}

QRect Script::globalRect() const
{
    QPoint p = mapToGlobal(QPoint(0, 0));
    QRect rc = rect();
    rc.moveTopLeft(p);
    return rc;
}

void Script::startPlay()
{
    m_isPlaying = true;
    m_timeBegin = QDateTime::currentDateTime();
    m_count = 0;
    m_runTimer->start();
    ui->toolButtonPlay->setIcon(QIcon(":/Script/Script/stop.png"));
    ui->labelState->setText("状态：运行中...");
    gScriptData.startScript();
}

void Script::stopPlay()
{
    m_isPlaying = false;
    m_runTimer->stop();
    ui->toolButtonPlay->setIcon(QIcon(":/Script/Script/play.png"));
    ui->labelState->setText("状态：已停止");
    gScriptData.stopScript();
}

void Script::onRecordTimer()
{
    if (m_tempCmd.size() != 2 && m_tempCmd.size() != 4) {
        qMsWarning() << "无效操作, size:" << m_tempCmd.size();
        m_tempCmd.clear();
        m_delayTimer.restart();
        return;
    }
    m_tempCmd.setTimeline(m_timeline.elapsed());
    gScriptData.appendCommand(&m_tempCmd);
    ui->listWidget->addItem(m_tempCmd.text());
    ui->listWidget->scrollToBottom();
    m_tempCmd.clear();
}

void Script::onScriptRunOnce()
{
    m_count++;
    ui->labelCount->setText(QString("执行次数：%1").arg(m_count));
}

void Script::onRunTimer()
{
    //
    qint64 seconds = m_timeBegin.secsTo(QDateTime::currentDateTime());
    int day = seconds / (3600 * 24);
    int day2 = seconds % (3600 * 24);
    int hour = day2 / 3600;
    int hour2 = day2 % 3600;
    int min = hour2 / 60;
    int sec = hour2 % 60;
    QString str = QString("运行时间：%1天%2时%3分%4秒")
                      .arg(day, 2, 10, QLatin1Char('0'))
                      .arg(hour, 2, 10, QLatin1Char('0'))
                      .arg(min, 2, 10, QLatin1Char('0'))
                      .arg(sec, 2, 10, QLatin1Char('0'));
    ui->labelTime->setText(str);
}

void Script::onScriptIndexChanged(int index)
{
    ui->listWidget->setCurrentRow(index);
}

void Script::on_toolButtonRecord_clicked()
{
    if (isPlaying()) {
        stopPlay();
    }

    if (isRecording()) {
        //停止录制
        ui->toolButtonRecord->setIcon(QIcon(":/Script/Script/record2.png"));
        m_isRecording = false;

        ui->labelState->setText("状态：录制完成");
        ui->toolButtonRecord->setToolTip("开始录制");
        ui->toolButtonPlay->setEnabled(true);
    } else {
        //开始录制
        ui->toolButtonRecord->setIcon(QIcon(":/Script/Script/record1.png"));
        m_isRecording = true;
        m_timeline.start();
        m_delayTimer.start();

        gScriptData.clearCommand();
        ui->listWidget->clear();

        ui->labelState->setText("状态：录制中...");
        ui->toolButtonRecord->setToolTip("停止录制");
        ui->toolButtonPlay->setEnabled(false);
    }
}

void Script::on_toolButtonPlay_clicked()
{
    if (isPlaying()) {
        stopPlay();
    } else {
        startPlay();
    }
}
