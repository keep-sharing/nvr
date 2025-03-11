#ifndef IMAGEPAGEROI_H
#define IMAGEPAGEROI_H

#include "abstractimagepage.h"
#include <QMap>
#include "DrawView.h"
#include "DrawSceneRoi.h"

namespace Ui {
class ImagePageRoi;
}

class ImagePageRoi : public AbstractImagePage
{
    enum MaskColumn
    {
        ColumnID,
        ColumnName,
        ColumnEnable,
        ColumnDelete,
    };
    Q_OBJECT

public:
    explicit ImagePageRoi(QWidget *parent = nullptr);
    ~ImagePageRoi();

    virtual void initializeData(int channel) override;
    virtual void processMessage(MessageReceive *message) override;

protected:
    void hideEvent(QHideEvent *event);

private slots:
    void closeWait();
    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();
    void on_pushButtonDeleteAll_clicked();
    void on_checkBox_enable_clicked(bool checked);
    void on_comboBox_VideoStream_activated();
    void onTableItemClicked(int row, int column);
	
    void on_pushButtonAdd_clicked();

    void on_pushButtonClear_clicked();

private:
    void ON_RESPONSE_FLAG_GET_ROI(MessageReceive *message);
    void ON_RESPONSE_FLAG_DELETE_ROI_AREA(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM(MessageReceive *message);

    void setSettingEnable(bool enable);

    void saveRoiInfo();
    void setMaskNewVersion(bool enable);
    void refreshTable();
    void setAreaData();
    void setTableEnabledStatus();
    void apply();	
private:
    Ui::ImagePageRoi *ui;

    DrawView *m_drawView = nullptr;
    DrawSceneRoi *m_drawScene = nullptr;

    int m_channel;
    QList<int> m_copyChannelList;
    set_image_roi m_roiMask;
    int m_sdkVersion;
    int m_maxArea;
    int m_currentRow;
    QPixmap m_uncheckedPixmap;
    QPixmap m_checkedPixmap;
};

#endif // IMAGEPAGEROI_H
