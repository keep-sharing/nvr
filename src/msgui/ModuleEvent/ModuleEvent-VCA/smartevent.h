#ifndef SMARTEVENT_H
#define SMARTEVENT_H

#include "AbstractSettingPage.h"

class BaseSmartWidget;

namespace Ui {
class SmartEvent;
}

class SmartEvent : public AbstractSettingPage {
    Q_OBJECT

    enum SmartItem {
        Item_RegionEntrance,
        Item_RegionExiting,
        Item_AdvancedMotionDetection,
        Item_TamperDeterction,
        Item_LineCrossing,
        Item_Loitering,
        Item_HumanDetection,
        Item_ObjectLeftRemoved,
        Item_Settings,
        Item_None
    };

public:
    explicit SmartEvent(QWidget *parent = nullptr);
    ~SmartEvent();

    void initializeData() override;
    void dealMessage(MessageReceive *message) override;

    void setApplyButtonEnable(bool enable);

protected:
    void hideEvent(QHideEvent *event) override;

private slots:
    void onLanguageChanged() override;
    void onTabBarClicked(int index);

    void onShowMessage(QString message);
    void onHideMessage();

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_pushButtonCopy_clicked();

private:
    Ui::SmartEvent *ui;

    SmartItem m_currentItemType = Item_None;
    QMap<SmartItem, BaseSmartWidget *> m_pageMap;
};

#endif // SMARTEVENT_H
