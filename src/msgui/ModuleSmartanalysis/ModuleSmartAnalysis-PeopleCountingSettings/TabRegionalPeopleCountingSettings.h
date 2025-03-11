#ifndef TABREGIONALPEOPLECOUNTINGSETTINGS_H
#define TABREGIONALPEOPLECOUNTINGSETTINGS_H

#include "AbstractSettingTab.h"
#include "msqtvca.h"
#include "pushbuttoneditstate.h"

class DrawSceneObjectSize;
class GraphicsMultiRegionScene;
class ActionRegionPeopleCounting;
class EffectiveTimePeopleCounting;

struct MsIpcRegionalPeople;

namespace Ui {
class TabRegionalPeopleCountingSettings;
}

class TabRegionalPeopleCountingSettings : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabRegionalPeopleCountingSettings(QWidget *parent = 0);
    ~TabRegionalPeopleCountingSettings();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_REGIONAL_PEOPLE(MessageReceive *message);
    void ON_RESPONSE_FLAG_UPDATE_REGIONAL_ACTION(MessageReceive *message);

    int currentRegionIndex() const;
    void updateEnableState();
    void clearSettings();

    bool saveRegionInfo(int index);

    void showDebugInfo(const QString &title);

    void clearCopyInfo();

private slots:
    void onLanguageChanged() override;

    void onChannelButtonClicked(int index);

    //object
    void onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType);

    void on_radioButtonMinLayout_clicked(bool checked);
    void on_radioButtonMaxLayout_clicked(bool checked);

    void onLineEditMinWidthEditingFinished();
    void onLineEditMinHeightEditingFinished();
    void onLineEditMaxWidthEditingFinished();
    void onLineEditMaxHeightEditingFinished();

    void on_pushButtonObjectSizeEdit_clicked();
    void on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state);
    void on_pushButtonObjectSizeReset_clicked();

    //region
    void onDrawRegionConflicted();

    void on_comboBoxRegion_indexSet(int index);
    void on_checkBoxEnable_checkStateSet(int state);

    void on_pushButtonRegionEdit_clicked();
    void on_pushButtonRegionEdit_stateSet(PushButtonEditState::State state);
    void on_pushButtonRegionSetAll_clicked();
    void on_pushButtonRegionDelete_clicked();

    void on_pushButtonEditAction_clicked();

    //
    void updateDynamicDisplay(int type, int channel);

    //
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

    void on_pushButtonCopy_clicked();

    void on_pushButtonRegionalPeopleCountingSchedule_clicked();

private:
    Ui::TabRegionalPeopleCountingSettings *ui;

    int m_currentChannel = 0;
    int m_currentRegion = -1;
    MsIpcRegionalPeople *m_regionInfo = nullptr;

    DrawSceneObjectSize *m_objScene = nullptr;
    GraphicsMultiRegionScene *m_regionScene = nullptr;

    ActionRegionPeopleCounting *m_action = nullptr;
    EffectiveTimePeopleCounting *m_effectiveTime = nullptr;

    quint64 m_copyFlags;
    int m_objectSizeType;
};

#endif // TABREGIONALPEOPLECOUNTINGSETTINGS_H
