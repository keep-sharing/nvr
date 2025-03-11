#ifndef SMARTSEARCHCONTROL_H
#define SMARTSEARCHCONTROL_H

#include "BasePlayback.h"
#include "SearchEventBackup.h"

class SmartSearchDrawRegion;

class SmartSearchControl : public BasePlayback {
    Q_OBJECT

    enum State {
        StateNone,
        StateMotion
    };

public:
    explicit SmartSearchControl(QWidget *parent = nullptr);

    static SmartSearchControl *instance();

    bool isSmartSearchMode();

    void showSmartSearch();
    void closeSmartSearch();

    void manualSeek(const QDateTime &dateTime);
    void research();
    void changePlayDirection();

    void stepForward();
    void stepBackward();

    bool hasRecord();

    int mode() const;
    void setMode(int newMode);

    QList<resp_search_event_backup> backupList() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;

private:
    bool isSmartSearchEnable();

    void getNextMotionTime(qint64 current, int type, qint64 &begin, qint64 &end);
    void getPreviousMotionTime(qint64 current, int type, qint64 &begin, qint64 &end);
    void clearCurrentMotionTime();
    void clearNextMotionTime();

signals:
    void modeChanged(int mode);

public slots:
    void onPlaybackRealTime(QDateTime dateTime);

private slots:
    void onDrawFinished();
    void onSearchFinished(int channel);

private:
    QEventLoop m_eventLoop;

    int m_mode = 0;

    char m_region[300];
    SmartSearchDrawRegion *m_drawMotion = nullptr;

    State m_state = StateNone;
    QMap<qint64, int> m_backupMap;

    qint64 m_currentMotionStartTime;
    qint64 m_currentMotionEndTime;
    qint64 m_nextMotionStartTime;
    qint64 m_nextMotionEndTime;

    bool m_isSearchFinished = false;
    bool m_fakerPause = false;
    bool m_isSeeked = false;

    SearchEventBackup *m_search = nullptr;
};

#endif // SMARTSEARCHCONTROL_H
