#ifndef PAGEALARMINPUT_H
#define PAGEALARMINPUT_H

#include "AbstractSettingPage.h"

namespace Ui {
class PageAlarmInput;
}

class PageAlarmInput : public AbstractSettingPage {
    Q_OBJECT

public:
    enum TAB_TYPE {
        TAB_NVR_ALARM_INPUT,
        TAB_CAMERA_ALARM_INPUT
    };

    explicit PageAlarmInput(QWidget *parent = nullptr);
    ~PageAlarmInput();

    void initializeData() override;

private slots:
    void onLanguageChanged();
    void onTabClicked(int index);

private:
    Ui::PageAlarmInput *ui;
};

#endif // PAGEALARMINPUT_H
