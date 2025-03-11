#ifndef DEVICESEARCH_H
#define DEVICESEARCH_H

#include "abstractcamerapage.h"
#include "SearchCameraEdit.h"
#include "SearchCameraEditMulti.h"
#include "SearchCameraAdd.h"
#include "SearchCameraAddMulti.h"
#include "SearchCameraActivate.h"
#include "CameraEdit.h"
#include "camerakey.h"

Q_DECLARE_METATYPE(resq_search_ipc)

namespace Ui {
class DeviceSearch;
}

class DeviceSearch : public AbstractCameraPage
{
    Q_OBJECT

public:
    enum DeviceColumn
    {
        ColumnCheck,
        ColumnNumber,
        ColumnIP,
        ColumnEdit,
        ColumnStatus,
        ColumnPort,
        ColumnProtocol,
        ColumnNIC,
        ColumnMAC,
        ColumnFirmware,
        ColumnModel,
        ColumnVendor,
        ColumnSN
    };
    enum PoeColumn
    {
        PoeColumnCheck,
        PoeColumnChannel,
        PoeColumnEdit,
        PoeColumnStatus,
        PoeColumnIP,
        PoeColumnPort,
        PoeColumnProtocol,
        PoeColumnMAC,
        PoeColumnFirmware,
        PoeColumnModel,
        PoeColumnVendor,
        PoeColumnSN
    };

    explicit DeviceSearch(QWidget *parent = 0);
    ~DeviceSearch();

    void initializeData() override;

    void setShowInWizard();
    void setShowInLiveView();

    void processMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_SEARCH_IPC(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_CAMERA_CHALLENGE(MessageReceive *message);

private:
    void initializeProtocol();
    void initializeNIC();

    void updateCameraMap(const req_set_ipcaddr &ipcAddr);
    void showCameraList();
    void makeMacformatInfo(char *inMac, int inlen, char *outMac);

private slots:
    void onLanguageChanged();

    void onTableItemClicked(int row, int column);
    void onTableHeaderChecked(bool checked);

    void onTablePoeItemClicked(int row, int column);

    void onActiveCameraChanged(QString password);

    void on_pushButton_search_clicked();
    void on_comboBox_protocol_activated(int index);
    void on_comboBox_nic_activated(int index);

    void on_pushButton_add_clicked();
    void on_pushButton_back_clicked();
    void on_pushButton_activate_clicked();
    void on_pushButton_ipEdit_clicked();

private:
    Ui::DeviceSearch *ui;

    bool m_isShowInWizard = false;

    network m_dbNetwork;
    QMap<int, ipc_protocol> m_protocolMap;

    //要搜索哪些协议，一个一个搜
    QList<int> m_searchProtocolList;

    //搜索到的
    QMap<CameraKey, resq_search_ipc> m_cameraMap;   //search

    //已经添加的所有IP map
    QMap<QString, int> m_existIpMap;   //key: ip

    //Repeat Ip
    QMap<QString, QString> m_cameraMapAllIp;
    QMap<QString, QString> m_cameraMapRepeatIp;

    //选中的行
    QList<int> m_checkedRowList;

    SearchCameraEdit *m_searchCameraEdit = nullptr;
    SearchCameraEditMulti *m_searchCameraEditMulti = nullptr;
    SearchCameraAdd *m_searchCameraAdd = nullptr;
    SearchCameraAddMulti *m_searchCameraAddMulti = nullptr;
    SearchCameraActivate *m_searchCameraActivate = nullptr;

    //
    CameraEdit *m_cameraEdit = nullptr;

    struct camera *m_camera_array = nullptr;
};

#endif // DEVICESEARCH_H
