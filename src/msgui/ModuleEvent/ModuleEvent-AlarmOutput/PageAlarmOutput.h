#ifndef PAGEALARMOUTPUT_H
#define PAGEALARMOUTPUT_H

#include "AbstractSettingPage.h"

namespace Ui {
class PageAlarmOutput;
}

class PageAlarmOutput : public AbstractSettingPage
{
    Q_OBJECT

public:
    enum TAB_TYPE
    {
        TAB_NVR_ALARM_OUTPUT,
        TAB_CAMERA_ALARM_OUTPUT
    };

    explicit PageAlarmOutput(QWidget *parent = nullptr);
    ~PageAlarmOutput();

    void initializeData() override;

private slots:
    void onLanguageChanged();
    void onTabClicked(int index);

private:
    Ui::PageAlarmOutput *ui;
};

#endif // PAGEALARMOUTPUT_H
