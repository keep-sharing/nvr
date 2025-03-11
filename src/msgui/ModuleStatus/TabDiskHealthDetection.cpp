#include "TabDiskHealthDetection.h"
#include "ui_TabDiskHealthDetection.h"
#include "DrawView.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsDisk.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyFileSystemDialog.h"
#include "centralmessage.h"
#include "qglobal.h"
#include <QDebug>

TabDiskHealthDetection::TabDiskHealthDetection(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabDiskHealthDetection)
{
    ui->setupUi(this);
    m_waitting = new MsWaitting(this);

    struct device_info device;
    db_get_device(SQLITE_FILE_NAME, &device);
    ui->comboBoxDisk->beginEdit();
    int maxHMDiskCnt;
    if (device.prefix[0] == '8') {
        maxHMDiskCnt = 9;
    } else {
        maxHMDiskCnt = 5;
    }
    for (int i = 0; i < maxHMDiskCnt - 1; i++) {
        QString diskName = QString("SATA%1").arg(i + 1);
        ui->comboBoxDisk->addItem(diskName, i + 1);
    }
    if (device.prefix[0] == '8') {
        ui->comboBoxDisk->addItem("eSATA", 25);
    }

    m_drawView = new DrawView(this);
    m_drawScene = new DrawSceneDiskHealth(this);
    m_drawView->setScene(m_drawScene);
    ui->gridLayoutResult->addWidget(m_drawView);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    m_timer->setInterval(2000);
    qDebug() << "TabDiskHealthDetection init";

    onLanguageChanged();
}

TabDiskHealthDetection::~TabDiskHealthDetection()
{
    delete ui;
}

void TabDiskHealthDetection::initializeData()
{
    ui->comboBoxDisk->setCurrentIndex(0);
    refreshData();
}

void TabDiskHealthDetection::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_DISK_HEALTH_MANAGEMENT_DATA:
        ON_RESPONSE_FLAG_GET_DISK_HEALTH_MANAGEMENT_DATA(message);
        break;
    case RESPONSE_FLAG_GET_DISK_HEALTH_MANAGEMENT_LOG:
        ON_RESPONSE_FLAG_GET_DISK_HEALTH_MANAGEMENT_LOG(message);
        break;
    }
}

void TabDiskHealthDetection::filterMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabDiskHealthDetection::refreshData()
{
    m_requestCount = 30;
    onTimer();
}

void TabDiskHealthDetection::clearInfo()
{
    m_drawScene->showDiskHealthMap(NULL);
    ui->labelTemperatureStatus->setText("N/A");
    ui->labelHealthStatus->setText("N/A");
    ui->toolButtonTemperatureStatus->hide();
    ui->toolButtonTemperatureStatus_2->hide();
    ui->widgetNotSupport->hide();
    ui->widgetHealthButton->show();
    ui->widgetHealthSatus->show();
    ui->widgetExpend->show();
    ui->pushButtonExport->setEnabled(false);
    ui->checkBoxEnable->setChecked(false);
}

void TabDiskHealthDetection::showEvent(QShowEvent *event)
{
    m_requestCount = 30;
    m_timer->start();
    QWidget::showEvent(event);
}

void TabDiskHealthDetection::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void TabDiskHealthDetection::ON_RESPONSE_FLAG_GET_DISK_HEALTH_MANAGEMENT_DATA(MessageReceive *message)
{
    if (message->isNull()) {
        clearInfo();
        //m_waitting->//closeWait();
        return;
    }
    struct disk_health_data_t *info = (struct disk_health_data_t *)message->data;
    m_hasDisk = info->hasDisk;
    if (!info->hasDisk) {
        clearInfo();
        //m_waitting->//closeWait();
        ui->checkBoxEnable->setChecked(info->HMEnable);
        on_checkBoxEnable_clicked(info->HMEnable);
        return;
    }
    m_drawScene->showDiskHealthMap(info->temperatureList);
    if (info->temperatureStatus == 1) {
        ui->labelTemperatureStatus->setText(GET_TEXT("STATUS/177002", "Warning!The working temperature of the disk is too high."));
        ui->toolButtonTemperatureStatus->setIcon(QIcon(":/status/status/diskWarning.png"));
    } else if (info->temperatureStatus == 2) {
        ui->labelTemperatureStatus->setText(GET_TEXT("STATUS/177003", "Warning!The working temperature of the disk is too low."));
        ui->toolButtonTemperatureStatus->setIcon(QIcon(":/status/status/diskWarning.png"));
    } else {
        ui->labelTemperatureStatus->setText(GET_TEXT("SMARTEVENT/146002", "Normal"));
        ui->toolButtonTemperatureStatus->setIcon(QIcon(":/status/status/diskNormal.png"));
    }
    ui->toolButtonTemperatureStatus->show();
    ui->toolButtonTemperatureStatus_2->show();
    m_HMStatus = info->HMStatus;
    m_supportHM = info->supportHM;
    ui->checkBoxEnable->setChecked(info->HMEnable);
    on_checkBoxEnable_clicked(info->HMEnable);
    //m_waitting->//closeWait();
}

void TabDiskHealthDetection::ON_RESPONSE_FLAG_GET_DISK_HEALTH_MANAGEMENT_LOG(MessageReceive *message)
{
    //m_waitting->//closeWait();
    int result = 1;
    if (message->isNull()) {
        result = 0;
    } else {
        char *logData = (char *)message;
        if (logData[0] == '\0') {
            result = 0;
        }
    }
    if (result == 1) {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145000", "Backup successfully."));
    } else {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145002", "Failed to backup."));
    }
    ui->pushButtonExport->clearHover();
}

void TabDiskHealthDetection::onLanguageChanged()
{
    ui->labelDisk->setText(GET_TEXT("STATUS/177010", "Disks"));
    ui->labelTip1->setText(GET_TEXT("STATUS/177001", "Smart Temperature Curve"));
    ui->labelTip2->setText(GET_TEXT("STATUS/177005", "Disk Early Warning"));
    ui->labelStatus1->setText(GET_TEXT("MENU/10006", "Status") + ":");
    ui->labelStatus2->setText(GET_TEXT("MENU/10006", "Status") + ":");
    ui->labelNotSupport->setText(GET_TEXT("STATUS/177011", "The Disk Early Warning function only support for the Skyhawk disk."));
    ui->pushButtonExport->setText(GET_TEXT("STATUS/177008", "Disk Files Export"));
    ui->labelExport->setText("(" + GET_TEXT("STATUS/177009", "Disk Files Export") + ")");
    ui->labelEnable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->checkBoxEnable->setText("");
    ui->labelEnableTip->setText("(" + GET_TEXT("STATUS/177016", "Enabling this feature may enhance IO, there is a risk of video interruption.") + ")");
}

void TabDiskHealthDetection::on_comboBoxDisk_activated(int index)
{
    Q_UNUSED(index)
    refreshData();
}

void TabDiskHealthDetection::on_pushButtonExport_clicked()
{
    QString dirPath = MyFileSystemDialog::instance()->getOpenDirectory();
    if (dirPath.isEmpty()) {
        return;
    }
    //m_waitting->//showWait();
    REQ_DISKHM_LOG_S log;
    memset(&log, 0, sizeof(REQ_DISKHM_LOG_S));
    log.port = ui->comboBoxDisk->currentIntData();
    snprintf(log.exportPath, sizeof(log.exportPath), "%s", dirPath.toStdString().c_str());
    sendMessage(REQUEST_FLAG_GET_DISK_HEALTH_MANAGEMENT_LOG, &log, sizeof(REQ_DISKHM_LOG_S));
}

void TabDiskHealthDetection::onTimer()
{
    m_requestCount++;
    if (m_requestCount > 30) {
        //m_waitting->//showWait();
        int port = ui->comboBoxDisk->currentIntData();
        Q_UNUSED(port)
        m_requestCount = 0;
    }
}
void TabDiskHealthDetection::saveData()
{
    struct disk_health_data_t disk;
    disk.port = ui->comboBoxDisk->currentIntData();
    disk.HMEnable = static_cast<MF_BOOL>(ui->checkBoxEnable->isChecked());
    Q_UNUSED(disk)
}

void TabDiskHealthDetection::on_checkBoxEnable_clicked(bool checked)
{
    if (checked && m_hasDisk) {
        if (!m_supportHM) {
            ui->widgetHealthButton->hide();
            ui->widgetHealthSatus->hide();
            ui->widgetExpend->hide();
            ui->widgetNotSupport->show();
        } else {
            ui->widgetNotSupport->hide();
            ui->widgetHealthButton->show();
            ui->widgetHealthSatus->show();
            ui->widgetExpend->show();
            ui->pushButtonExport->setEnabled(true);
            if (m_HMStatus == 101) {
                ui->labelHealthStatus->setText(GET_TEXT("STATUS/177007", "Warning!The SATA interface is detected to be unstable, please check."));
                ui->toolButtonTemperatureStatus_2->setIcon(QIcon(":/status/status/diskWarning.png"));
            } else if (m_HMStatus == 102 || m_HMStatus == 105) {
                ui->labelHealthStatus->setText(GET_TEXT("STATUS/177006", "Warning!The disk vibration is detected, please make sure that your NVR is placed smoothly."));
                ui->toolButtonTemperatureStatus_2->setIcon(QIcon(":/status/status/diskWarning.png"));
            } else {
                ui->labelHealthStatus->setText(GET_TEXT("SMARTEVENT/146002", "Normal"));
                ui->toolButtonTemperatureStatus_2->setIcon(QIcon(":/status/status/diskNormal.png"));
            }
        }
    } else {
        ui->labelHealthStatus->setText("N/A");
        ui->toolButtonTemperatureStatus_2->hide();
        ui->widgetNotSupport->hide();
        ui->widgetHealthButton->show();
        ui->widgetHealthSatus->show();
        ui->widgetExpend->show();
        ui->pushButtonExport->setEnabled(false);
    }
}
