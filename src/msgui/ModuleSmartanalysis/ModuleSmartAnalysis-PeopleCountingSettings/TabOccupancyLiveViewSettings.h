#ifndef TABOCCUPANCYLIVEVIEWSETTINGS_H
#define TABOCCUPANCYLIVEVIEWSETTINGS_H

#include "AbstractSettingTab.h"
#include <QEventLoop>

extern "C" {
#include "msdb.h"
}

class ActionOccupancy;
class OccupancyGroupSettings;

namespace Ui {
class TabOccupancyLiveViewSettings;
}

class TabOccupancyLiveViewSettings : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabOccupancyLiveViewSettings(QWidget *parent = 0);
    ~TabOccupancyLiveViewSettings();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_UPDATE_PEOPLECNT_SETTING(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_PEOPLECNT_LIVEVIEW_ACTION(MessageReceive *message);

    void showGroupComboBox(int defaultGroup);
    void cacheGroupSettings(int index);

private slots:
    void onLanguageChanged() override;

    void on_pushButtonGroupSettings_clicked();
    void on_comboBoxGroup_indexSet(int index);
    void on_comboBoxPeopleCounting_indexSet(int index);
    void on_pushButtonLiveViewCountingReset_clicked();
    void on_pushButtonNvrCountingReset_clicked();
    void on_comboBoxLiveViewCountingAutoReset_indexSet(int index);
    void on_pushButtonAlarmAction_clicked();

    void on_toolButtonNote1_clicked(bool checked);
    void on_toolButtonNote2_clicked(bool checked);

    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::TabOccupancyLiveViewSettings *ui;

    int m_currentGroupIndex = -1;

    people_cnt_info m_settings_info;

    OccupancyGroupSettings *m_groupSettings = nullptr;

    QEventLoop m_eventLoop;
    ActionOccupancy *m_action = nullptr;
};

#endif // TABOCCUPANCYLIVEVIEWSETTINGS_H
