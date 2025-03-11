#ifndef ACTIONPAGEALARMOUTPUT_H
#define ACTIONPAGEALARMOUTPUT_H

#include "ActionPageAbstract.h"
#include "AlarmKey.h"

namespace Ui {
class ActionPageAlarmOutput;
}

class ActionPageAlarmOutput : public ActionPageAbstract {
    Q_OBJECT

    enum AlarmOutColumn {
        AlarmOutCheck,
        AlarmOutNo,
        AlarmOutName,
        AlarmOutEdit,
        AlarmOutDelete
    };

public:
    explicit ActionPageAlarmOutput(QWidget *parent = nullptr);
    ~ActionPageAlarmOutput();

protected:
    int loadData() override;
    int saveData() override;

private:
    void updateTriggerAlarmOutputTable();

private slots:
    void onLanguageChanged() override;
    void onTableViewAlarmOutputClicked(int row, int column);

private:
    Ui::ActionPageAlarmOutput *ui;

    QMap<AlarmKey, bool> m_alarmoutMap;
};

#endif // ACTIONPAGEALARMOUTPUT_H
