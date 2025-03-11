#ifndef IMAGEPAGEMASK_H
#define IMAGEPAGEMASK_H

#include "abstractimagepage.h"
#include <QEventLoop>
#include "DrawView.h"
#include "DrawSceneMask.h"
#include "DrawItemMaskControl.h"

namespace Ui {
class ImagePageMask;
}

class ImagePageMask : public AbstractImagePage
{
    enum MaskColumn
    {
        ColumnID,
        ColumnName,
        ColumnType,
        ColumnEnable,
        ColumnEdit,
        ColumnDelete,
    };
    Q_OBJECT

public:
    explicit ImagePageMask(QWidget *parent = nullptr);
    ~ImagePageMask();

    virtual void initializeData(int channel) override;
    virtual void processMessage(MessageReceive *message) override;

protected:
    void hideEvent(QHideEvent *event) override;

private:
    void ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_PRIVACY_MASK(MessageReceive *message);
    void ON_RESPONSE_FLAG_DELETE_PRIVACY_MASK(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_PRIVACY_MASK(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message);
    
    void clearSetting();
    void setSettingEnable(bool enable);

    void setMaskEnable(bool enable);
    void setMaskNewVersion(bool enable);
    void refreshTable();
    void setTableEnabledStatus();
    void isSupportMosaic();

    int waitForSaveMask(int channel);
    QString getType(int type);

private slots:
    void onLanguageChanged();

    void onTableItemClicked(int row, int column);

    void on_checkBox_enable_clicked(bool checked);
    void on_pushButtonDeleteAll_clicked();
    void on_pushButtonAdd_clicked();
    void on_pushButtonClear_clicked();

    void on_comboBox_type_activated(int index);

    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_comboBoxRegionType_activated(int index);

private:
    Ui::ImagePageMask *ui;

    QGraphicsScene *m_drawMaskScene = nullptr;

    DrawItemMaskControl *m_drawItemMask = nullptr;
    int m_currentChannel;
    int m_currentRow;
    resp_privacy_mask m_privacy_mask;
    QEventLoop m_eventLoop;
    int m_maskNUM;
};

#endif // IMAGEPAGEMASK_H
