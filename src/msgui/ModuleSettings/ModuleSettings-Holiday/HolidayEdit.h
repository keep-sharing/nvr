#ifndef HOLIDAYEDIT_H
#define HOLIDAYEDIT_H

#include "BaseShadowDialog.h"

struct holiday;

namespace Ui {
class HolidayEdit;
}

class HolidayEdit : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit HolidayEdit(QWidget *parent = nullptr);
    ~HolidayEdit();

    void showHolidayEdit(holiday *day);

private:
    bool dealInput();

private slots:
    void onLanguageChanged();

    void on_comboBox_style_activated(int index);

    void on_comboBox_startMonth_currentIndexChanged(int index);
    void on_comboBox_endMonth_currentIndexChanged(int index);

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();
    void editFinish();

private:
    Ui::HolidayEdit *ui;

    holiday *m_cacheData = nullptr;
    holiday *m_sourceData = nullptr;
};

#endif // HOLIDAYEDIT_H
