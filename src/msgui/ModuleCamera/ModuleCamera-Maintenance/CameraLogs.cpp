#include "CameraLogs.h"
#include "ui_CameraLogs.h"

#include "CameraStatusWidget.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"

CameraLogs::CameraLogs(QWidget *parent) :
    AbstractSettingTab(parent),
    ui(new Ui::CameraLogs)
{
    ui->setupUi(this);
    ui->comboBoxMainType->beginEdit();
    ui->comboBoxMainType->clear();
    ui->comboBoxMainType->addItem("All Types",CameraAllTypes);
    ui->comboBoxMainType->addItem("Event",CameraEvent);
    ui->comboBoxMainType->addItem("Operation",CameraOperation);
    ui->comboBoxMainType->addItem("Information",CameraInformation);
    ui->comboBoxMainType->addItem("Exception",CameraException);
    ui->comboBoxMainType->addItem("Smart",CameraSmart);
    ui->comboBoxMainType->endEdit();

    ui->lineEditExport->setEnabled(false);

    //initialize table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("CHANNELMANAGE/30009", "Channel Name");
    headerList << GET_TEXT("MENU/10006", "Status");
    headerList << GET_TEXT("COMMON/1033", "IP Address");
    headerList << GET_TEXT("FISHEYE/12016", "Channel ID");
    headerList << GET_TEXT("CHANNELMANAGE/30014", "Protocol");
    headerList << GET_TEXT("CHANNELMANAGE/30027", "MAC");
    headerList << GET_TEXT("CHANNELMANAGE/30054", "Firmware Version");
    headerList << GET_TEXT("CHANNELMANAGE/30029", "Model");
    headerList << "Export Progress";
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(headerChecked(bool)), this, SLOT(onTableHeaderChecked(bool)));
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortType(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnIP, SortFilterProxyModel::SortIP);
    ui->tableView->setSortType(ColumnStatus, SortFilterProxyModel::SortInt);
    //column width
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnChannel, 90);
    ui->tableView->setColumnWidth(ColumnName, 150);
    ui->tableView->setColumnWidth(ColumnStatus, 150);
    ui->tableView->setColumnWidth(ColumnIP, 150);
    ui->tableView->setColumnWidth(ColumnChannelId, 100);
    ui->tableView->setColumnWidth(ColumnMAC, 140);
    ui->tableView->setColumnWidth(ColumnFirmware, 180);
    ui->tableView->setColumnWidth(ColumnModel, 180);
}

CameraLogs::~CameraLogs()
{
    delete ui;
}

void CameraLogs::initializeData()
{

}

void CameraLogs::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message);
}

bool CameraLogs::isCloseable()
{
    if (ui->pushButtonBackup->isEnabled()) {
        return true;
    }
    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38010", "Exit will stop all unfinished progress, continue?"));
    if (result == MessageBox::Cancel) {
        return false;
    }

    if (ui->pushButtonBackup->isEnabled()) {
        return true;
    }
    ////showWait();
    return false;
}

void CameraLogs::onLanguageChanged()
{

}

void CameraLogs::onTableItemClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
}

void CameraLogs::onTableHeaderChecked(bool checked)
{
    Q_UNUSED(checked)
}

void CameraLogs::on_pushButtonBrowse_clicked()
{
    ui->pushButtonBrowse->clearUnderMouse();
    ui->pushButtonBrowse->clearFocus();

    QString filePath = MyFileSystemDialog::instance()->getOpenFileInfo();
    if (filePath.isEmpty()) {
        return;
    }
    ui->lineEditExport->setText(filePath);
}

void CameraLogs::on_pushButtonBackup_clicked()
{

}

void CameraLogs::on_pushButtonBack_clicked()
{
    if (isCloseable()) {
        back();
    }
}

void CameraLogs::on_comboBoxMainType_currentIndexChanged(int index)
{
    ui->comboBoxSubType->beginEdit();
    ui->comboBoxSubType->clear();
    ui->comboBoxSubType->addItem("All Types",0);
    switch (index) {
    case CameraEvent:
        ui->comboBoxSubType->addItem("Motion Detection Start",1);
        ui->comboBoxSubType->addItem("Motion Detection Stop",2);
        ui->comboBoxSubType->addItem("External Input Alarm",3);
        ui->comboBoxSubType->addItem("External Output Alarm",4);
        ui->comboBoxSubType->addItem("Audio Input Alarm",5);
        break;
    case CameraOperation:
        ui->comboBoxSubType->addItem("Reboot Remotely",1);
        ui->comboBoxSubType->addItem("Login Remotely",2);
        ui->comboBoxSubType->addItem("Logout Remotely",3);
        ui->comboBoxSubType->addItem("Upgrade Remotely",4);
        ui->comboBoxSubType->addItem("Config Remotely",5);
        ui->comboBoxSubType->addItem("PTZ Control Remotely",6);
        ui->comboBoxSubType->addItem("Playback Remotely",7);
        ui->comboBoxSubType->addItem("Export Profile Remotely",8);
        ui->comboBoxSubType->addItem("Import Profile Remotely",9);
        ui->comboBoxSubType->addItem("Reset Remotely",10);
        ui->comboBoxSubType->addItem("RTSP Session Start",11);
        ui->comboBoxSubType->addItem("RTSP Session Stop",12);
        ui->comboBoxSubType->addItem("Video Param Set Remotely",13);
        ui->comboBoxSubType->addItem("Image Param Se Remotely",14);
        break;
    case CameraInformation:
        ui->comboBoxSubType->addItem("Record Start",1);
        ui->comboBoxSubType->addItem("Record Stop",2);
        ui->comboBoxSubType->addItem("Reset Locally",3);
        ui->comboBoxSubType->addItem("System Restart",4);
        ui->comboBoxSubType->addItem("RTSP Over Maximum",5);
        ui->comboBoxSubType->addItem("RTSP IP Limit",6);
        ui->comboBoxSubType->addItem("IR-CUT Off",7);
        ui->comboBoxSubType->addItem("IP-CUN On",8);
        ui->comboBoxSubType->addItem("IR LED Off",9);
        ui->comboBoxSubType->addItem("IP LED On",10);
        ui->comboBoxSubType->addItem("White LED Off",11);
        ui->comboBoxSubType->addItem("White LED On",12);
        break;
    case CameraException:
        ui->comboBoxSubType->addItem("Storage Full",1);
        ui->comboBoxSubType->addItem("Network Disconnected",2);
        ui->comboBoxSubType->addItem("IP Address Conflict",3);
        ui->comboBoxSubType->addItem("Record Failed",4);
        ui->comboBoxSubType->addItem("Snapshot Failed",5);
        ui->comboBoxSubType->addItem("AV Server Died",6);
        ui->comboBoxSubType->addItem("RTSP Server Died",7);
        ui->comboBoxSubType->addItem("Web Server Died",8);
        ui->comboBoxSubType->addItem("SD Card Uninitialized",9);
        ui->comboBoxSubType->addItem("SD Card Error",10);
        ui->comboBoxSubType->addItem("No SD Card",11);
        ui->comboBoxSubType->addItem("Upload to FTP Failed",12);
        ui->comboBoxSubType->addItem("Send Email Failed",13);
        break;
    case CameraSmart:
        ui->comboBoxSubType->addItem("Region Entrance",1);
        ui->comboBoxSubType->addItem("Region Exit",2);
        ui->comboBoxSubType->addItem("Loitering",3);
        ui->comboBoxSubType->addItem("Advanced Motion Detection",4);
        ui->comboBoxSubType->addItem("Object Left",5);
        ui->comboBoxSubType->addItem("Object Removed",6);
        ui->comboBoxSubType->addItem("Cross L1 A->B",7);
        ui->comboBoxSubType->addItem("Cross L1 B->A",8);
        ui->comboBoxSubType->addItem("Cross L2 A->B",9);
        ui->comboBoxSubType->addItem("Cross L2 B->A",10);
        ui->comboBoxSubType->addItem("Cross L3 A->B",11);
        ui->comboBoxSubType->addItem("Cross L3 B->A",12);
        ui->comboBoxSubType->addItem("Cross L4 A->B",13);
        ui->comboBoxSubType->addItem("Cross L4 B->A",14);
        ui->comboBoxSubType->addItem("Tamper Detection",15);
        ui->comboBoxSubType->addItem("Human Detection",16);
        ui->comboBoxSubType->addItem("People Detection",17);
        break;
    default:
        break;
    }
    ui->comboBoxSubType->endEdit();
}
