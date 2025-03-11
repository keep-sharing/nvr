#ifndef SMARTSPEEDPANEL_H
#define SMARTSPEEDPANEL_H

#include "BasePlayback.h"
#include "DateTimeRange.h"
#include "SmartSpeedDebug.h"
#include <QWidget>

namespace Ui {
class SmartSpeedPanel;
}

class SmartSpeedPanel : public BasePlayback {
    Q_OBJECT

    enum State {
        StateNone,
        StateGeneral,
        StateEvent
    };

public:
    explicit SmartSpeedPanel(QWidget *parent = nullptr);
    ~SmartSpeedPanel();

    void initializeData();
    void closeSmartSpeed();
    void clearTimeInfo();
    void closePanel();

    //开启smart search时临时禁用
    void temporarilyDisableForSmartSearch();
    bool isTemporarilyDisableForSmartSearch() const;
    void resumeSmartSpeed();

    bool isSmartPlaySpeedEnable() const;
    bool isSkipGeneralVideo() const;
    PLAY_SPEED speedForGeneral() const;
    PLAY_SPEED speedForEvent() const;

    void manualSeek(const QDateTime &dateTime);
    void updateEventRecord();
    void changePlayDirection();

    void stepForward();
    void stepBackward();
    //事件过滤开启只显示事件时使用。等同于开启智能播放
    void startEventPlayBack();
    void closeEventPlayBack();

    void dealMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_OPEN(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_PAGE(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message) override;

    void dealCommonTimeInfo(resp_search_common_backup *common_array, int count);
    void dealEventTimeInfo(resp_search_event_backup *event_array, int count);
    void insertGeneralTime(const resp_search_common_backup &backup);
    void insertEventTime(const resp_search_common_backup &backup);
    void insertEventTime(const resp_search_event_backup &backup);
    void getNextGeneralTime(qint64 current, int type, qint64 &begin, qint64 &end);
    void getPreviousGeneralTime(qint64 current, int type, qint64 &begin, qint64 &end);
    void getNextEventTime(qint64 current, int type, qint64 &begin, qint64 &end);
    void getPreviousEventTime(qint64 current, int type, qint64 &begin, qint64 &end);

    void enterEventSpeed();
    void enterGeneralSpeed();

    void fakerPauseGeneralLastSec();
    void fakerPauseGeneralFirstSec();
    void fakerPauseEventLastSec();
    void fakerPauseEventFirstSec();

    void clearEventTime();
    void clearGeneralTime();

    QString timeString(qint64 sec1, qint64 sec2);

signals:
    void smartSpeedStateChanged(int state);

public slots:
    void onSmartSpeedPanelButtonClicked(int x, int y);

private slots:
    void onLanguageChanged();
    void onRefreshDebugInfo();
    void onPlaybackModeChanged(MsPlaybackType mode);

    void onPlaybackRealTime(QDateTime dateTime);

    void on_comboBox_smartPlaySpeed_currentIndexChanged(int index);
    void on_comboBox_skipGeneralVideo_currentIndexChanged(int index);

    void on_pushButton_apply_clicked();
    void on_pushButton_cancel_clicked();

    void on_pushButton_debug_clicked();

private:
    Ui::SmartSpeedPanel *ui;

    State m_state = StateNone;
    bool m_fakerPause = false;
    bool m_isSeeked = false;

    int m_currentChannel = -1;

    bool m_smartPlaySpeedEnable = false;
    bool m_skipGeneralVideo = true;
    PLAY_SPEED m_speedForGeneral = PLAY_SPEED_1X;
    PLAY_SPEED m_speedForEvent = PLAY_SPEED_1X;

    QMap<qint64, int> m_generalSecMap;
    QMap<qint64, int> m_eventSecMap;
    qint64 m_currentGeneralStartSec = -1;
    qint64 m_currentGeneralEndSec = -1;
    qint64 m_nextGeneralStartSec = -1;
    qint64 m_nextGeneralEndSec = -1;
    qint64 m_currentEventStartSec = -1;
    qint64 m_currentEventEndSec = -1;
    qint64 m_nextEventStartSec = -1;
    qint64 m_nextEventEndSec = -1;

    SmartSpeedDebug *m_debug = nullptr;

    //临时禁用
    bool m_isTemporarilyDisabled = false;
};

#endif // SMARTSPEEDPANEL_H
