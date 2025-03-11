#include "PlaybackCut.h"
#include "ui_PlaybackCut.h"

#include "MsLanguage.h"
#include "PlaybackTimeLine.h"

#include <QDialog>

PlaybackTimeLine *PlaybackCut::s_playbackTimeLine = nullptr;

PlaybackCut::PlaybackCut(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::PlaybackCut)
{
    ui->setupUi(this);
    setWindowModality(Qt::NonModal);

    PlaybackTimeLine::s_playbackCut = this;

    connect(ui->timeEdit_start, SIGNAL(timeChanged(QTime)), this, SLOT(onStartTimeChanged(QTime)));
    connect(ui->timeEdit_end, SIGNAL(timeChanged(QTime)), this, SLOT(onEndTimeChanged(QTime)));

    ui->toolButton_close->setToolTip(GET_TEXT("COMMON/1004", "Cancel"));
    ui->toolButton_save->setToolTip(GET_TEXT("COMMON/1036", "Save"));
}

PlaybackCut::~PlaybackCut()
{
    delete ui;
}

int PlaybackCut::showCut()
{
    show();
    return 0;
}

void PlaybackCut::closeCut()
{
    on_toolButton_close_clicked();
}

void PlaybackCut::setRange(const QDateTime &begin, const QDateTime &end)
{
    m_beginDateTime = begin;
    m_endDateTime = end;
    setBeginTimeEdit(m_beginDateTime);
    setEndTimeEdit(m_endDateTime);
}

void PlaybackCut::setBeginTime(const QDateTime &dateTime)
{
    m_beginDateTime = dateTime;
    if (m_beginDateTime >= s_playbackTimeLine->timeLineEndDateTime()) {
        m_beginDateTime = s_playbackTimeLine->timeLineEndDateTime().addSecs(-1);
    }
    if (m_endDateTime.isNull()) {
        m_endDateTime = m_beginDateTime.addSecs(1);
    }
    QTime endTime = ui->timeEdit_end->time();
    m_endDateTime.setTime(endTime);
    if (m_beginDateTime >= m_endDateTime) {
        setEndTime(m_beginDateTime.addSecs(1));
    }
    //
    setBeginTimeEdit(m_beginDateTime);
}

void PlaybackCut::setEndTime(const QDateTime &dateTime)
{
    m_endDateTime = dateTime;
    if (m_endDateTime <= s_playbackTimeLine->timeLineBeginDateTime()) {
        m_endDateTime = s_playbackTimeLine->timeLineBeginDateTime().addSecs(1);
    }
    if (m_beginDateTime.isNull()) {
        m_beginDateTime = m_endDateTime.addSecs(-1);
    }
    QTime beginTime = ui->timeEdit_start->time();
    m_beginDateTime.setTime(beginTime);
    if (m_endDateTime <= m_beginDateTime) {
        setBeginTime(m_endDateTime.addSecs(-1));
    }
    //
    setEndTimeEdit(m_endDateTime);
}

QDateTime PlaybackCut::beginDateTime()
{
    return m_beginDateTime;
}

QDateTime PlaybackCut::endDateTime()
{
    return m_endDateTime;
}

bool PlaybackCut::isMoveToCenter()
{
    return false;
}

void PlaybackCut::setBeginTimeEdit(const QDateTime &dateTime)
{
    disconnect(ui->timeEdit_start, SIGNAL(timeChanged(QTime)), this, SLOT(onStartTimeChanged(QTime)));
    ui->timeEdit_start->setTime(dateTime.time());
    connect(ui->timeEdit_start, SIGNAL(timeChanged(QTime)), this, SLOT(onStartTimeChanged(QTime)));
}

void PlaybackCut::setEndTimeEdit(const QDateTime &endTime)
{
    Q_UNUSED(endTime)

    disconnect(ui->timeEdit_end, SIGNAL(timeChanged(QTime)), this, SLOT(onEndTimeChanged(QTime)));
    ui->timeEdit_end->setTime(m_endDateTime.time());
    connect(ui->timeEdit_end, SIGNAL(timeChanged(QTime)), this, SLOT(onEndTimeChanged(QTime)));
}

void PlaybackCut::onStartTimeChanged(const QTime &time)
{
    m_beginDateTime.setTime(time);
    if (m_beginDateTime >= m_endDateTime) {
        m_beginDateTime = m_endDateTime.addSecs(-1);
        setBeginTimeEdit(m_beginDateTime);
    }
    s_playbackTimeLine->setCutBeginDateTime(m_beginDateTime);
}

void PlaybackCut::onEndTimeChanged(const QTime &time)
{
    m_endDateTime.setTime(time);
    if (m_endDateTime <= m_beginDateTime) {
        m_endDateTime = m_beginDateTime.addSecs(1);
        setEndTimeEdit(m_endDateTime);
    }
    if (m_endDateTime.date() > m_beginDateTime.date()) {
        m_endDateTime.setTime(QTime(0, 0, 0));
        setEndTimeEdit(m_endDateTime);
    }
    s_playbackTimeLine->setCutEndDateTime(m_endDateTime);
}

void PlaybackCut::on_toolButton_close_clicked()
{
    s_playbackTimeLine->closeCut();
    close();
    //m_eventLoop.exit(QDialog::Rejected);
}

void PlaybackCut::on_toolButton_save_clicked()
{
    s_playbackTimeLine->closeCut();
    close();
    //m_eventLoop.exit(QDialog::Accepted);
}
