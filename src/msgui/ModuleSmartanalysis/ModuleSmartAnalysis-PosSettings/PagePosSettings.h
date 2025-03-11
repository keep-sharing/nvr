#ifndef PAGEPOSSETTINGS_H
#define PAGEPOSSETTINGS_H

#include "AbstractSettingPage.h"
#include "GraphicsPosDisplayRegion.h"
#include "PosData.h"

extern "C" {
#include "msdb.h"
#include "msdefs.h"
}

class ActionPos;
class EffectiveTimePos;

namespace Ui {
class PosSettings;
}

class PagePosSettings : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PagePosSettings(QWidget *parent = nullptr);
    ~PagePosSettings();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SET_POS_SETTING(MessageReceive *message);

    bool saveCurrentData();

private slots:
    void onLanguageChanged() override;

    void onSceneMouseDragging();

    void on_comboBoxPosNo_indexSet(int index);
    void on_comboBoxPos_indexSet(int index);
    void on_comboBoxPosProtocol_indexSet(int index);
    void on_pushButtonConnectionModeSettingsEdit_clicked();
    void on_comboBoxDisplayChannel_indexSet(int index);
    void on_pushButtonDisplayRegionReset_clicked();
    void on_pushButtonPrivacySettingsEdit_clicked();
    void on_pushButtonEffectiveTimeEdit_clicked();
    void on_pushButtonActionEdit_clicked();

    void updateEnableState();

    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::PosSettings *ui;

    int m_currentPosNo = -1;
    Db_POS_CONFIG m_lastPosConfigs[MAX_POS_CLIENT];
    Db_POS_CONFIG m_currentPosConfigs[MAX_POS_CLIENT];
    GraphicsPosDisplayRegion *m_drawRegion = nullptr;

    EffectiveTimePos *m_effectiveTime = nullptr;
    ActionPos *m_action = nullptr;
};

#endif // PAGEPOSSETTINGS_H
