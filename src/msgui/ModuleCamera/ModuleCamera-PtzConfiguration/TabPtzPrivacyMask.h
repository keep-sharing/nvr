#ifndef TABPTZPRIVACYMASK_H
#define TABPTZPRIVACYMASK_H

#include <QWidget>
#include <QEventLoop>
#include "ptzbasepage.h"
#include "DrawSceneMask.h"
#include "DrawItemPTZMaskControl.h"
extern "C"
{
#include "msg.h"
}

class DrawView;
class DrawScenePtzMask;

namespace Ui {
class PtzPrivacyMaskPage;
}

class TabPtzPrivacyMask : public PtzBasePage
{
    Q_OBJECT

    enum MaskColumn
    {
        ColumnID,
        ColumnName,
        ColumnType,
        ColumnEnable,
        ColumnRatio,
        ColumnEdit,
        ColumnAreaEdit,
        ColumnDelete
    };

public:
    explicit TabPtzPrivacyMask(QWidget *parent = nullptr);
    ~TabPtzPrivacyMask();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_PRIVACY_MASK(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_PRIVACY_MASK(MessageReceive *message);
    void ON_RESPONSE_FLAG_DELETE_PRIVACY_MASK(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM(MessageReceive *message);

    int waitForGetPrivacyMask();
    int waitForSaveMask(int channel);
    int waitForDeleteMask(int channel, int id);

    void setSettingEnable(bool enable);
    void setFunctionEnable(bool enable);
    void clearSetting();

    QString typeString(int type);
    void showMaskTable();
    void addPrivacyMask();
    void isSupportMosaic();

protected:
    void hideEvent(QHideEvent *event) override;

private slots:
    void onLanguageChanged();

    void onChannelGroupClicked(int channel);
    void onTableItemClicked(int row, int column);

    void on_checkBox_enable_checkStateSet(int checked);

    void onPushButtonAddClicked();
    void on_pushButton_clear_clicked();
    void on_pushButton_clearAll_clicked();

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_comboBoxRegionType_activated(int index);
    void on_comboBoxMaskColor_activated(int index);

private:
    Ui::PtzPrivacyMaskPage *ui;

    QEventLoop m_eventLoop;
    DrawItemPTZMaskControl *m_drawItemMask = nullptr;

    resp_privacy_mask m_privacy_mask;
    mask_area_ex m_editingArea;
    bool m_isReadyAdd = false;

    int m_maskNUM;
    QList<int> m_maskList;
    Uint64 m_clearMask = 0;
};

#endif // TABPTZPRIVACYMASK_H
