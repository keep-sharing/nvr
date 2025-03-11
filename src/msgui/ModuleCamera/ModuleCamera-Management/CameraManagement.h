#ifndef CAMERAMANAGEMENT_H
#define CAMERAMANAGEMENT_H

#include "CameraEditTabSettings.h"
#include "CameraStreamJsonData.h"
#include "MsCameraModel.h"
#include "abstractcamerapage.h"
#include "authentication.h"
#include "itembuttonwidget.h"
#include <QModelIndex>
#include <QMutex>

class CameraEdit;
class MessageReceive;

namespace Ui {
class CameraManagement;
}

class CameraManagement : public AbstractCameraPage {
    Q_OBJECT

    struct BatchInfo {
        int channel = -1;
        resp_get_ipc_param param;
        resp_http_common_param main_common;
        resp_http_common_param sub_common;

        BatchInfo()
        {
            param.chnid = -1;
            main_common.chnid = -1;
            sub_common.chnid = -1;
        }
    };

public:
    enum CameraColumn {
        ColumnCheck,
        ColumnChannel,
        ColumnName,
        ColumnEdit,
        ColumnDelete,
        ColumnStatus,
        ColumnIP,
        ColumnChannelId,
        ColumnPort,
        ColumnProtocol,
        ColumnMAC,
        ColumnFirmware,
        ColumnModel,
        ColumnSN,
        ColumnVendor
    };
    enum CameraTabIdx {
        TabCameraManagement = 0,
        TabBatchSetting,
    };

    explicit CameraManagement(QWidget *parent = 0);
    ~CameraManagement();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_NETWORK_BANDWIDTH(MessageReceive *message);
    void ON_RESPONSE_FLAG_TEST_IPCCONNECT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCPARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPCPARAM_BATCH(MessageReceive *message);
    void ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPCPARAM_EX(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_FRAME_RESOLUTION(MessageReceive *message);
    void ON_RESPONSE_FLAG_REMOVE_IPC(MessageReceive *message);

protected:
    void hideEvent(QHideEvent *event) override;

private:
    void deleteCamera(int channel);
    bool isInputValid();
    void testIpcConnect();
    void refreshUnusedChannelList();

    void clearBatchSettings();
    void clearCameraManagement();

    void updateBatchSettings(const BatchInfo &info);
    BatchInfo getBatchInfo(int channel);
    void lineEditIPShow(bool enable);
    void updateFrameRateMain(int maxFrameRate, int currentFrameRate, MsCameraModel cameraModel);

private slots:
    void onLanguageChanged();

    //tab
    void onTabClicked(int index);
    //table
    void onTableItemClicked(int row, int column);
    void onTableHeaderChecked(bool checked);
    void onTableDoubleClicked(QModelIndex);

    void getBatchParameters();

    void refreshData();
    void onAuthChanged();
    void onEditChanged();

    void on_comboBox_channel_activated(int index);
    void on_comboBox_protocol_activated(int index);
    void on_pushButton_test_clicked();
    void on_pushButton_add_clicked();

    void on_comboBox_secondaryStream_activated(int index);
    void on_comboBox_maxFrameRate_main_activated(int index);
    void on_comboBox_maxFrameRate_sub_activated(int index);
    void on_pushButton_apply_clicked();

    void on_pushButton_authentication_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_refresh_clicked();
    void on_pushButton_back_clicked();

    void on_lineEdit_ip_textEdited(const QString &arg1);
    void on_lineEdit_primary_textEdited(const QString &arg1);
    void on_lineEdit_secondary_textEdited(const QString &arg1);

    void on_comboBoxSmartStreamMain_activated(int index);
    void on_comboBoxSmartStreamSub_activated(int index);

    void on_comboBox_frameSize_main_indexSet(int index);
    void on_comboBox_frameSize_main_activated(int index);
    void on_comboBox_frameSize_sub_indexSet(int index);
    void on_comboBox_frameSize_sub_activated(int index);

    void on_comboBox_videoCodec_main_activated(int index);
    void on_comboBox_videoCodec_sub_activated(int index);

    void on_comboBox_transportProtocol_activated(int index);

  private:
    Ui::CameraManagement *ui;

    int m_currentRow = -1;
    int m_currentTab = -1;
    QMap<int, resq_get_ipcdev> m_ipcdevMap;
    QMap<int, BatchInfo> m_batchInfoMap;

    resp_get_ipc_param m_temp_ipc_param;
    resp_http_common_param m_temp_common_param;

    IPC_CONN_RES m_ipcTestResult;

    QList<int> m_channelCheckedList;
    bool m_isBatchAll = false;

    QMap<int, int> m_readyDeleteMap; //key: row

    CameraEdit *m_cameraEdit = nullptr;
    Authentication *m_authentication = nullptr;

    QEventLoop m_eventLoop;

    QString m_state;

    //fisheye
    ms_ipc_fisheye_info m_fisheye_info;

    //protocol
    QString m_msddns;
    QString m_ipAddress;
    QString m_primary;
    QString m_secondary;
    IPC_PROTOCOL m_perProtocol;

    //max frame size
    QByteArray m_jsonData;
    CameraStreamJsonData *m_cameraStreamJsonData = nullptr;
    resp_get_ipc_param m_ipc_param;
    QString m_mainStream;
    QString m_SubStream;
};

#endif // CAMERAMANAGEMENT_H
