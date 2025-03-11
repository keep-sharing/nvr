#ifndef DAYNIGHTSCHEEDIT_H
#define DAYNIGHTSCHEEDIT_H

#include "abstractimagepage.h"
#include "BaseShadowDialog.h"

struct image_day_night;
struct ImageDaynightScene;

namespace Ui {
class DayNightScheEdit;
}

class DayNightScheEdit : public BaseShadowDialog {
    Q_OBJECT

public:
    enum scheType {
        NIGHT,
        DAY,
        OTHER
    };
    explicit DayNightScheEdit(bool isWhite, QWidget *parent = 0);
    ~DayNightScheEdit();
    void initDayNightEditInfo(image_day_night *data, int id, int fullcolorSupport);
    void initDayNightEditInfoMulti(ImageDaynightScene *data, int id, int type);
    void setExposureCtrl(int type);

signals:
    void sigEditSche();

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    void slotTranslateUi();
    void initShutter();

private:
    Ui::DayNightScheEdit *ui;
    int m_channel;
    int m_scheId;
    int m_exposureCtrl = -1;
    image_day_night *m_info = nullptr;
    ImageDaynightScene *m_infoMulti = nullptr;
    int m_type = 0; 
};

#endif // DAYNIGHTSCHEEDIT_H
