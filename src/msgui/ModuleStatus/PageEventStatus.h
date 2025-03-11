#ifndef PAGEEVENTSTATUS_H
#define PAGEEVENTSTATUS_H

#include "AbstractSettingPage.h"
#include "AlarmKey.h"
#include "MsLanguage.h"
#include "tableview.h"

#include <QTimer>

extern "C" {
#include "msg.h"
}

namespace Ui {
class EventStatus;
}

class PageEventStatus : public AbstractSettingPage {
    Q_OBJECT

    //
    struct AlarmInfo {
        int alarmCount = 0;
        QString alarmName;
        int alarmType = 0;
        int status = 0;
        int delay = 0;

        QString alarmTypeString() const
        {
            QString text;
            switch (alarmType) {
            case 0:
                text = GET_TEXT("ALARMIN/52005", "NO");
                break;
            case 1:
                text = GET_TEXT("ALARMIN/52006", "NC");
                break;
            }
            return text;
        }
        QString delayString() const
        {
            if (delay > 0) {
                return QString("%1s").arg(delay);
            } else {
                return GET_TEXT("EVENTSTATUS/63011", "Manually Clear");
            }
        }
    };

public:
    explicit PageEventStatus(QWidget *parent = 0);
    ~PageEventStatus();

    void initializeData() override;

    bool canAutoLogout() override;

    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VAC_SUPPORT_ALL(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_EVENT_STATUS(MessageReceive *message);

    void resizeEvent(QResizeEvent *event) override;
    void hideEvent(QHideEvent *) override;

private slots:
    void onLanguageChanged() override;

    void onTimerUpdate();

private:
    void initializeTableCmaera();
    void initializeTableAlarm();
    void initializeTableSmartEvent();
    void initializeTablePeopleCount();

    void updateTableCameraStatus(TableView *table, int row, int column, QString ip, bool status);

private slots:
    void onTabClicked(int index);

    void on_pushButton_back_clicked();

private:
    Ui::EventStatus *ui;

    QTimer *m_timer = nullptr;
    ms_vca_support_all m_vcaSupportInfo;

    QMap<AlarmKey, AlarmInfo> m_alarminMap;
    QMap<AlarmKey, AlarmInfo> m_alarmoutMap;
};

#endif // PAGEEVENTSTATUS_H
