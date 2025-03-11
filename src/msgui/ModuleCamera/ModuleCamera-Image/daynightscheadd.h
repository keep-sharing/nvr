#ifndef DAYNIGHTSCHEADD_H
#define DAYNIGHTSCHEADD_H

#include "abstractimagepage.h"
#include "BaseShadowDialog.h"

extern "C"
{
#include "msg.h"
}

namespace Ui {
class DayNightScheAdd;
}

class DayNightScheAdd : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit DayNightScheAdd(bool isWhite, QWidget *parent = 0);
    ~DayNightScheAdd();
    void initAddDayNight(image_day_night *info);
	void setExposureCtrl(int type);
	
signals:
	void sigAddSche();
	
private:
	void slotTranslateUi();
	void initShutter();
	
private slots:
	void on_pushButton_ok_add_clicked();
	void on_pushButton_cancel_add_clicked();

private:
    Ui::DayNightScheAdd *ui;
	int m_channel;
	int m_scheId;
	int m_exposureCtrl = -1;
    image_day_night *m_info = nullptr;
};

#endif // DAYNIGHTSCHEADD_H
