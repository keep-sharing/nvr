#ifndef PLAYBACKCUT_H
#define PLAYBACKCUT_H

#include "BaseShadowDialog.h"

#include <QDateTime>
#include <QEventLoop>

class PlaybackTimeLine;

namespace Ui {
class PlaybackCut;
}

class PlaybackCut : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit PlaybackCut(QWidget *parent = nullptr);
    ~PlaybackCut();

    static PlaybackTimeLine *s_playbackTimeLine;

    int showCut();
    void closeCut();

    void setRange(const QDateTime &begin, const QDateTime &end);
    void setBeginTime(const QDateTime &dateTime);
    void setEndTime(const QDateTime &dateTime);

    QDateTime beginDateTime();
    QDateTime endDateTime();

protected:
    bool isMoveToCenter();

private:
    void setBeginTimeEdit(const QDateTime &dateTime);
    void setEndTimeEdit(const QDateTime &endTime);

private slots:
    void onStartTimeChanged(const QTime &time);
    void onEndTimeChanged(const QTime &time);

    void on_toolButton_close_clicked();
    void on_toolButton_save_clicked();

private:
    Ui::PlaybackCut *ui;

    QEventLoop m_eventLoop;

    QDateTime m_beginDateTime;
    QDateTime m_endDateTime;
};

#endif // PLAYBACKCUT_H
