#ifndef TABFACECAPTURE_H
#define TABFACECAPTURE_H

#include "AbstractSettingTab.h"
#include "DrawItemFaceCapture.h"
#include <QEventLoop>

extern "C" {

}

class ActionFace;
class EffectiveTimeFace;
class FaceCaptureSettings;

namespace Ui {
class TabFaceCapture;
}

class TabFaceCapture : public AbstractSettingTab {
    enum MaskColumn {
        ColumnID,
        ColumnName,
        ColumnEnable,
        ColumnDelete,
    };
    Q_OBJECT

public:
    explicit TabFaceCapture(QWidget *parent = nullptr);
    ~TabFaceCapture();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_FACE_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FACE_CONFIG(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_FACE_CONFIG(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(MessageReceive *message);

    void hideMessage();
    void showNotConnectedMessage();
    void showNotSupportMessage();
    void updateEnableState();
    void clearSetting();
    void refreshTable();
    void setTableEnabledStatus();
    bool isPTZCamera();

private slots:
    void onLanguageChanged() override;
    void onChannelButtonClicked(int index);
    void onTableItemClicked(int row, int column);

    void on_comboBoxRegionEdit_currentIndexChanged(int index);

    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();
    void on_checkBoxEnable_stateChanged(int arg1);

    void on_pushButtonFaceCaptureSetting_clicked();
    void on_horizontalSliderMinSize_valueChanged(int value);
    void on_horizontalSliderMinSize_sliderMoved(int position);

    void on_pushButtonSetAll_clicked();
    void on_pushButtonRegionDelete_clicked();

    void on_pushButtonAdd_clicked();
    void on_pushButtonClear_clicked();
    void on_pushButtonDeleteAll_clicked();

    void onDrawPolygonConflicted();

    void on_pushButtonEffective_clicked();
    void on_pushButtonAction_clicked();

private:
    Ui::TabFaceCapture *ui;

    int m_currentChannel = 0;
    bool m_isConnected = false;
    bool m_isSupported = -1;
    bool m_hasEnable = false;

    MsFaceConfig m_faceConfig;

    DrawItemFaceCapture *m_drawItem = nullptr;
    FaceCaptureSettings *m_faceSetting = nullptr;

    EffectiveTimeFace *m_effectiveTime = nullptr;
    ActionFace *m_action = nullptr;

    QEventLoop m_eventLoop;
};

#endif // TABFACECAPTURE_H
