#include "CameraDiagnoseInformation.h"
#include "ui_CameraDiagnoseInformation.h"
#include "CameraStatusWidget.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"

CameraDiagnoseInformation::CameraDiagnoseInformation(QWidget *parent) :
    AbstractSettingTab(parent),
    ui(new Ui::CameraDiagnoseInformation)
{
    ui->setupUi(this);

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

CameraDiagnoseInformation::~CameraDiagnoseInformation()
{
    delete ui;
}

void CameraDiagnoseInformation::initializeData()
{

}

void CameraDiagnoseInformation::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message);
}

bool CameraDiagnoseInformation::isCloseable()
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

void CameraDiagnoseInformation::onLanguageChanged()
{

}

void CameraDiagnoseInformation::on_pushButtonBrowse_clicked()
{
    ui->pushButtonBrowse->clearUnderMouse();
    ui->pushButtonBrowse->clearFocus();

    QString filePath = MyFileSystemDialog::instance()->getOpenFileInfo();
    if (filePath.isEmpty()) {
        return;
    }
    ui->lineEditDirectory->setText(filePath);
}

void CameraDiagnoseInformation::on_pushButtonBackup_clicked()
{

}

void CameraDiagnoseInformation::on_pushButtonBack_clicked()
{
    if (isCloseable()) {
        back();
    }
}
