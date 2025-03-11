#ifndef EFFECTIVETIMEABSTRACT_H
#define EFFECTIVETIMEABSTRACT_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class EffectiveTimeAbstract;
}

class EffectiveTimeAbstract : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit EffectiveTimeAbstract(QWidget *parent = nullptr);
    ~EffectiveTimeAbstract();

    void showEffectiveTime(int channel, int dataIndex = 0);
    void saveEffectiveTime();
    void saveEffectiveTime(int dataIndex);

    void clearCache();

protected:
    void onLanguageChanged();

    virtual QString titleText() const;
    virtual QString pushButtonEffectiveText() const;

    struct Data {
        smart_event_schedule schedule;
    };
    //如Camera Alarm Input, dataIndex() 代表Alarm Input No.
    int channel() const;
    int dataIndex() const;
    void setChannel(int channel);
     void setDataIndex(int dataIndex);
    //
    virtual bool holidayVisible();
    virtual schedule_day *schedule();
    virtual QColor scheduleColor() const = 0;
    virtual int scheduleType() const = 0;
    virtual schedule_day *readSchedule() = 0;
    virtual void saveSchedule() = 0;
    virtual void saveSchedule(int dataIndex);

private slots:
    void on_pushButtonEffective_clicked();
    void on_pushButtonErase_clicked();

    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

protected:
    QMap<int, Data> m_dataMap;
    Ui::EffectiveTimeAbstract *ui;
private:

    int m_channel = -1;
    int m_dataIndex = -1;
};

#endif // EFFECTIVETIMEABSTRACT_H
