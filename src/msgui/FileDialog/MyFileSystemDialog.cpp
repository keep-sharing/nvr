#include "MyFileSystemDialog.h"
#include "ui_MyFileSystemDialog.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "DeviceInfoWidget.h"
#include "FormatDialog.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "settingcontent.h"
#include "SubControl.h"
#include <QDir>
#include <QPainter>
#include <QPropertyAnimation>

MyFileSystemDialog *MyFileSystemDialog::s_fileSystemDialog = nullptr;

MyFileSystemDialog::MyFileSystemDialog(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::MyFileSystemDialog)
{
    ui->setupUi(this);
    setTitleWidget(ui->label_title);
    s_fileSystemDialog = this;

    //
    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    } else {
        qWarning() << QString("SettingContent load style failed.");
    }
    file.close();

    //
    m_animation = new QPropertyAnimation(this, "geometry");
    m_animation->setDuration(1000);
    m_animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(m_animation, SIGNAL(finished()), this, SLOT(close()));

    //
    connect(ui->tableView_file, SIGNAL(pathChanged(QString)), this, SLOT(onPathChanged(QString)));
    connect(ui->tableView_file, SIGNAL(currentFileInfo(QFileInfo)), this, SLOT(onCurrentFileInfo(QFileInfo)));

    //

    ui->comboBox_snapshotResolution->clear();
    ui->comboBox_snapshotResolution->addItem("Auto", 0);
    ui->comboBox_snapshotResolution->addItem("704*576", 1);
    ui->comboBox_snapshotResolution->addItem("640*360", 2);

    ui->widget_pathBar->hide();

    //
    ui->comboBoxPeopleCountingFileFormat->clear();
    ui->comboBoxPeopleCountingFileFormat->addItem("CSV", PeopleCounting_CSV);
    ui->comboBoxPeopleCountingFileFormat->addItem("PDF", PeopleCounting_PDF);
    ui->comboBoxPeopleCountingFileFormat->addItem("PNG", PeopleCounting_PNG);

    //
    m_watting = new MsWaitting(this);
    m_watting->setModal(true);
    m_watting->hide();

    onLanguageChanged();
}

MyFileSystemDialog::~MyFileSystemDialog()
{
    qDebug() << "MyFileSystemDialog::~MyFileSystemDialog()";
    s_fileSystemDialog = nullptr;
    delete ui;
}

MyFileSystemDialog *MyFileSystemDialog::instance()
{
    return s_fileSystemDialog;
}

QString MyFileSystemDialog::bytesString(qint64 bytes)
{
    // According to the Si standard KB is 1000 bytes, KiB is 1024
    // but on windows sizes are calculated by dividing by 1024 so we do what they do.
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;
    if (bytes >= tb)
        return QString("%1 TB").arg(QLocale().toString(qreal(bytes) / tb - 0.0004, 'f', 3));
    if (bytes >= gb)
        return QString("%1 GB").arg(QLocale().toString(qreal(bytes) / gb - 0.004, 'f', 2));
    if (bytes >= mb)
        return QString("%1 MB").arg(QLocale().toString(qreal(bytes) / mb - 0.04, 'f', 1));
    if (bytes >= kb)
        return QString("%1 KB").arg(QLocale().toString(bytes / kb));
    return QString("%1 bytes").arg(QLocale().toString(bytes));
}

void MyFileSystemDialog::initializeData()
{
    resize(1081, 686);
    move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), SubControl::instance()->logicalMainScreenGeometry()).topLeft());
    ui->widgetContent->show();

    ui->tableView_file->setFixedSize();

    ui->widget_file->hide();
    ui->widget_directory->hide();
    ui->widget_exportFile->hide();
    ui->widgetExportPos->hide();
    ui->widget_streamType->hide();
    ui->widget_format->hide();
    ui->widget_snapshotResolution->hide();
    ui->pushButton_export->hide();
    ui->pushButton_ok->hide();
    ui->widgetPeopleCounting->hide();

    //clear cache
    ui->comboBox_stream->setCurrentIndex(0);
    ui->comboBox_fileFormat->setCurrentIndex(0);
    ui->comboBox_snapshotResolution->setCurrentIndex(0);
    ui->comboBoxPeopleCountingFileFormat->setCurrentIndex(0);

}

QString MyFileSystemDialog::getOpenFileInfo()
{
    show();

    initializeData();
    ui->widget_file->show();
    ui->pushButton_ok->show();
    m_mode = ModeGetFile;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    ////m_watting->//showWait();

    int result = exec();
    if (result == QDialog::Accepted) {
        return ui->tableView_file->selectedFilePath();
    } else {
        return QString();
    }
}

/**
 * @brief MyFileSystemDialog::getOpenDirectory
 * /media/usb1_1/222
 * @return
 */
QString MyFileSystemDialog::getOpenDirectory()
{
    show();

    initializeData();
    ui->widget_directory->show();
    ui->pushButton_ok->show();
    m_mode = ModeGetDirectory;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    ////m_watting->//showWait();

    int result = exec();
    if (result == QDialog::Accepted) {
        return ui->tableView_file->currentPath();
    } else {
        return QString();
    }
}

QString MyFileSystemDialog::exportFile()
{
    show();
    ui->tableView_file->setFixedSize();

    initializeData();
    ui->widget_directory->show();
    ui->pushButton_export->show();
    m_mode = ModeNormalExport;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    ////m_watting->//showWait();

    const int &result = m_eventLoop.exec();
    if (result == QDialog::Accepted) {
        return ui->tableView_file->currentPath();
    } else {
        return QString();
    }
}

QString MyFileSystemDialog::exportVideo()
{
    show();
    ui->tableView_file->setFixedSize();

    initializeData();
    ui->widget_directory->show();
    ui->widget_format->show();
    ui->pushButton_export->show();
    m_mode = ModeExportVideo;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_watting->//showWait();

    int result = exec();
    if (result == QDialog::Accepted) {
        return ui->tableView_file->currentPath();
    } else {
        return QString();
    }
}

QString MyFileSystemDialog::exportVideoWithAnimate()
{
    show();
    ui->tableView_file->setFixedSize();

    initializeData();
    ui->widget_directory->show();
    ui->widget_format->show();
    ui->pushButton_export->show();
    m_mode = ModeExportVideoWithAnimate;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    ////m_watting->//showWait();

    int result = m_eventLoop.exec();
    if (result == QDialog::Accepted) {
        m_pixmap = QPixmap::grabWidget(this);
        ui->widgetContent->hide();
        //
        m_animation->setStartValue(geometry());
        QPoint p = SettingContent::instance()->globalDownloadPos();
        QRect rc2(0, 0, 3, 3);
        rc2.moveCenter(p);
        m_animation->setEndValue(rc2);
        m_animation->start();
        //
        return ui->tableView_file->currentPath();
    } else {
        return QString();
    }
}

QString MyFileSystemDialog::exportAnpr()
{
    show();
    ui->tableView_file->setFixedSize();

    initializeData();
    ui->widget_directory->show();
    ui->widget_exportFile->show();
    ui->widget_streamType->show();
    ui->widget_format->show();
    ui->pushButton_export->show();
    m_mode = ModeExportAnpr;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_watting->//showWait();

    int result = exec();
    if (result == QDialog::Accepted) {
        return ui->tableView_file->currentPath();
    } else {
        return QString();
    }
}

QString MyFileSystemDialog::exportPos()
{
    show();
    ui->tableView_file->setFixedSize();

    initializeData();
    ui->widget_directory->show();
    ui->widgetExportPos->show();
    ui->widget_format->show();
    ui->pushButton_export->show();
    m_mode = ModeExportPos;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_watting->//showWait();

    int result = exec();
    if (result == QDialog::Accepted) {
        return ui->tableView_file->currentPath();
    } else {
        return QString();
    }
}

QString MyFileSystemDialog::exportFace()
{
    show();
    ui->tableView_file->setFixedSize();

    initializeData();
    ui->widget_directory->show();
    ui->widget_exportFile->show();
    ui->widget_streamType->show();
    ui->widget_format->show();
    ui->pushButton_export->show();
    ui->checkBox_plateList->hide();
    m_mode = ModeExportAnpr;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_watting->//showWait();

    int result = exec();
    if (result == QDialog::Accepted) {
        return ui->tableView_file->currentPath();
    } else {
        return QString();
    }
}

QString MyFileSystemDialog::exportPicture()
{
    show();
    ui->tableView_file->setFixedSize();

    initializeData();
    ui->widget_directory->show();
    ui->widget_snapshotResolution->show();
    ui->pushButton_export->show();
    m_mode = ModeNormalExport;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_watting->//showWait();

    const int &result = m_eventLoop.exec();
    if (result == QDialog::Accepted) {
        return ui->tableView_file->currentPath();
    } else {
        return QString();
    }
}

QString MyFileSystemDialog::exportPictureWithAnimate()
{
    show();
    ui->tableView_file->setFixedSize();

    initializeData();
    ui->widget_directory->show();
    ui->widget_snapshotResolution->show();
    ui->pushButton_export->show();
    m_mode = ModeNormalExportWithAnimate;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_watting->//showWait();

    const int &result = m_eventLoop.exec();
    if (result == QDialog::Accepted) {
        m_pixmap = QPixmap::grabWidget(this);
        ui->widgetContent->hide();
        //
        m_animation->setStartValue(geometry());
        QPoint p = SettingContent::instance()->globalDownloadPos();
        QRect rc2(0, 0, 3, 3);
        rc2.moveCenter(p);
        m_animation->setEndValue(rc2);
        m_animation->start();
        //
        return ui->tableView_file->currentPath();
    } else {
        return QString();
    }
}

ExportPeopleCountingInfo MyFileSystemDialog::exportPeopleCounting()
{
    show();

    initializeData();
    ui->widget_directory->show();
    ui->widgetPeopleCounting->show();
    ui->pushButton_export->show();
    m_mode = ModeGetDirectory;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_watting->//showWait();

    const int &result = exec();

    ExportPeopleCountingInfo info;
    if (result == QDialog::Accepted) {
        info.path = ui->tableView_file->currentPath();
        info.fileFormat = static_cast<ExportPeopleCountingFileFormat>(ui->comboBoxPeopleCountingFileFormat->currentData().toInt());
    }
    return info;
}

void MyFileSystemDialog::showDialog()
{
    show();

    initializeData();
    ui->pushButton_ok->show();

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_watting->//showWait();

    exec();
}

QString MyFileSystemDialog::currentDeviceName()
{
    QString strDeviceName;
    if (m_currentDevice) {
        strDeviceName = m_currentDevice->deviceName();
    }
    return strDeviceName;
}

qint64 MyFileSystemDialog::currentDeviceFreeBytes()
{
    qint64 bytes = 0;
    if (m_currentDevice) {
        bytes = m_currentDevice->freeBytes();
    }
    return bytes;
}

int MyFileSystemDialog::currentDeviceFreeMegaBytes()
{
    return currentDeviceFreeBytes() >> 20;
}

int MyFileSystemDialog::currentDevicePort()
{
    int port = -1;
    if (m_currentDevice) {
        port = m_currentDevice->port();
    }
    return port;
}

int MyFileSystemDialog::anprExportType() const
{
    int type = 0;
    if (ui->checkBox_plateList->isChecked()) {
        type |= ANPR_PLATE_LIST;
    }
    if (ui->checkBox_picture->isChecked()) {
        type |= ANPR_PICTURE;
    }
    if (ui->checkBox_video->isChecked()) {
        type |= ANPR_VIDEO;
    }
    return type;
}

int MyFileSystemDialog::anprExportBackType() const
{
    int type = 0;
    if (ui->checkBox_plateList->isChecked()) {
        type |= ANPR_PLATE_LIST;
    }
    if (ui->checkBox_picture->isChecked()) {
        type |= ANPR_PICTURE;
    }
    if (ui->checkBox_video->isChecked()) {
        if (streamType() & (1 << FILE_TYPE_MAIN)) {
            type |= ANPR_VIDEO;
        }
        if (streamType() & (1 << FILE_TYPE_SUB)) {
            type |= ANPR_SUB_VIDEO;
        }
    }
    return type;
}

int MyFileSystemDialog::streamType() const
{
    return ui->comboBox_stream->currentData().toInt();
}

int MyFileSystemDialog::fileType() const
{
    return ui->comboBox_fileFormat->currentIndex();
}

int MyFileSystemDialog::pictureResolution() const
{
    return ui->comboBox_snapshotResolution->currentData().toInt();
}

QPoint MyFileSystemDialog::exportButtonPos()
{
    return m_exportPos;
}

bool MyFileSystemDialog::isExportAnprPlate() const
{
    return ui->checkBox_plateList->isChecked();
}

bool MyFileSystemDialog::isExportAnprVideo() const
{
    return ui->checkBox_video->isChecked();
}

bool MyFileSystemDialog::isExportAnprPicture() const
{
    return ui->checkBox_picture->isChecked();
}

void MyFileSystemDialog::clearAnprSelected()
{
    ui->checkBox_plateList->setChecked(false);
    ui->checkBox_video->setChecked(false);
    ui->checkBox_picture->setChecked(false);
    ui->checkBox_plateList->show();
}

int MyFileSystemDialog::exportPosFileType() const
{
    int type = 0;
    if (ui->checkBoxExportPosList->isChecked()) {
        type |= POS_LIST;
    }
    if (ui->checkBoxExportPosVideo->isChecked()) {
        type |= POS_VIDEO;
    }
    return type;
}

void MyFileSystemDialog::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    if (!isVisible()) {
        return;
    }

    switch (message->type()) {
    case RESPONSE_FLAG_UDISK_OFFLINE:
        ON_RESPONSE_FLAG_UDISK_OFFLINE(message);
        message->accept();
        break;
    }

    if (m_formatDialog) {
        m_formatDialog->dealMessage(message);
    }
}

void MyFileSystemDialog::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_EXPORT_DISK:
        ON_RESPONSE_FLAG_GET_EXPORT_DISK(message);
        break;
    }
}

void MyFileSystemDialog::ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message)
{
    //m_watting->//closeWait();

    struct resp_usb_info *usb_info_list = (struct resp_usb_info *)message->data;
    int count = message->header.size / sizeof(struct resp_usb_info);

    m_usbInfoMap.clear();
    for (int i = 0; i < count; ++i) {
        const resp_usb_info &usb_info = usb_info_list[i];
        if (!QString(usb_info.dev_path).isEmpty() && usb_info.type != DISK_TYPE_NAS && usb_info.bRec != true) {
            m_usbInfoMap.insert(usb_info.port, usb_info);
        }
    }
    updateDeviceList();
}

void MyFileSystemDialog::ON_RESPONSE_FLAG_UDISK_OFFLINE(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "MyFileSystemDialog::ON_RESPONSE_FLAG_UDISK_OFFLINE, data is null.";
        return;
    }
    int port = *((int *)message->data);
    if (m_usbInfoMap.contains(port)) {
        m_usbInfoMap.remove(port);
        updateDeviceList();
    }
}

bool MyFileSystemDialog::isDrawShadow()
{
    return m_animation->state() != QPropertyAnimation::Running;
}

void MyFileSystemDialog::hideEvent(QHideEvent *event)
{
    BaseShadowDialog::hideEvent(event);
}

void MyFileSystemDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (m_animation->state() == QPropertyAnimation::Running) {
        QPainter painter(this);
        painter.drawPixmap(rect(), m_pixmap);
    } else {
        BaseShadowDialog::paintEvent(event);
    }
}

void MyFileSystemDialog::updateDeviceList()
{
    m_currentDevice = nullptr;
    ui->label_path->clear();
    ui->lineEdit_fileName->clear();
    ui->lineEdit_directory->clear();
    ui->listWidget_device->clear();
    for (auto iter = m_usbInfoMap.constBegin(); iter != m_usbInfoMap.constEnd(); ++iter) {
        const resp_usb_info &usb_info = iter.value();
        QString devicePath(usb_info.dev_path);
        if (!devicePath.isEmpty()) {
            DeviceInfoWidget *widget = new DeviceInfoWidget(usb_info);
            widget->setDeviceName(usb_info.dev_name);
            widget->setDevicePath(devicePath);
            widget->setIsProtected(usb_info.bprotect);
            widget->setIsFormated(usb_info.bformat);
            widget->setFreeBytes(usb_info.free << 20); //MB to bytes
            ui->listWidget_device->addDeviceWidget(widget);
        }
        m_usbInfoMap.insert(usb_info.port, usb_info);
    }
    if (ui->listWidget_device->count() > 0) {
        ui->listWidget_device->setCurrentRow(0);
        on_listWidget_device_itemClicked(ui->listWidget_device->item(0));
    } else {
        ui->tableView_file->setRootIndexPath(QString());
        ui->tableView_file->setErrorString("No device found.");
    }
}

void MyFileSystemDialog::onLanguageChanged()
{
    ui->pushButton_newFolder->setText(GET_TEXT("COMMONBACKUP/100014", "New Folder"));
    ui->pushButton_format->setText(GET_TEXT("DISKMANAGE/72051", "Format"));
    ui->pushButton_refresh->setText(GET_TEXT("COMMON/1035", "Refresh"));

    ui->tableView_file->onLanguageChanged();

    ui->label_directory->setText(GET_TEXT("DISK/92006", "Directory"));
    ui->label_exportFile->setText(GET_TEXT("ANPR/103058", "Export File"));
    ui->checkBox_plateList->setText(GET_TEXT("ANPR/103059", "Plate List"));
    ui->checkBox_video->setText(GET_TEXT("ANPR/103060", "Video"));
    ui->checkBox_picture->setText(GET_TEXT("ANPR/103061", "Picture"));
    ui->label_videoStreamType->setText(GET_TEXT("ANPR/103062", "Video Stream Type"));
    ui->label_videoFileFormat->setText(GET_TEXT("COMMONBACKUP/100056", "Video File Format"));

    ui->pushButton_export->setText(GET_TEXT("ANPR/103052", "Export"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->label_title->setText(GET_TEXT("SYSTEMGENERAL/70005", "Device"));
    ui->label_fileName->setText(GET_TEXT("COMMONBACKUP/100055", "File Name"));
    ui->groupBox->setTitle(GET_TEXT("SYSTEMGENERAL/70005", "Device"));
    ui->groupBox_2->setTitle(GET_TEXT("LOG/64073", "Operation"));

    ui->labelExportFilePos->setText(GET_TEXT("ANPR/103058", "Export File"));
    ui->checkBoxExportPosList->setText(GET_TEXT("POS/130037", "POS List"));
    ui->checkBoxExportPosVideo->setText(GET_TEXT("ANPR/103060", "Video"));
    ui->labelPeopleCountingFileFormat->setText(GET_TEXT("COMMONBACKUP/100053", "File Format"));

    ui->comboBox_stream->clear();
    ui->comboBox_stream->addItem(GET_TEXT("IMAGE/37333", "Primary Stream"), 1 << FILE_TYPE_MAIN);
    ui->comboBox_stream->addItem(GET_TEXT("CHANNELMANAGE/30057", "Secondary Stream"), 1 << FILE_TYPE_SUB);
    ui->comboBox_stream->addItem(GET_TEXT("RECORDADVANCE/91020", "Primary+Secondary Stream"), (1 << FILE_TYPE_MAIN) | (1 << FILE_TYPE_SUB));
}

void MyFileSystemDialog::onPathChanged(const QString &path)
{
    if (m_usbInfoMap.isEmpty()) {
        ui->tableView_file->setErrorString("No device found.");
        return;
    }

    ui->label_path->setText(path);
    switch (m_mode) {
    case ModeGetFile: {
        break;
    }
    case ModeGetDirectory:
    case ModeNormalExport:
    case ModeExportVideo:
    case ModeExportAnpr:
    case ModeExportPos:
    case ModeNormalExportWithAnimate:
    case ModeExportVideoWithAnimate: {
        ui->lineEdit_directory->setText(path);
        break;
    }
    default:
        break;
    }
}

void MyFileSystemDialog::onCurrentFileInfo(const QFileInfo &fileInfo)
{
    switch (m_mode) {
    case ModeGetFile: {
        ui->lineEdit_fileName->clear();
        if (fileInfo.isFile()) {
            ui->lineEdit_fileName->setText(fileInfo.fileName());
        }
        break;
    }
    case ModeGetDirectory:
    case ModeNormalExport:
    case ModeExportVideo:
    case ModeExportAnpr:
    case ModeExportPos: {
        ui->lineEdit_directory->setText(fileInfo.path());
        break;
    }
    default:
        break;
    }
}

void MyFileSystemDialog::onNewFolder(const QString &name)
{
    QString path = ui->tableView_file->currentPath();
    QString trueName = name.trimmed();
    QFileInfo fileInfo(path + "/" + trueName);
    if (fileInfo.exists()) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100033", "Directory already exists!"));
        return;
    }
    bool ok = QDir(path).mkpath(trueName);
    if (!ok) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/151000", "Create folder failed."));
    }
    sync();
    ui->tableView_file->setRootIndexPath(path);
}

void MyFileSystemDialog::on_pushButton_ok_clicked()
{
    accept();
}

void MyFileSystemDialog::on_pushButton_cancel_clicked()
{
    switch (m_mode) {
    case ModeNormalExport:
        m_eventLoop.exit(QDialog::Rejected);
        break;
    case ModeNormalExportWithAnimate:
    case ModeExportVideoWithAnimate:
        m_eventLoop.exit(QDialog::Rejected);
        break;
    default:
        break;
    }
    ui->pushButton_cancel->clearFocus();
    reject();
}

void MyFileSystemDialog::on_pushButton_export_clicked()
{
    if (!m_currentDevice) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100035", "The current external device does not exist or is damaged, check the external device and refresh the list."));
        return;
    }

    switch (m_mode) {
    case ModeNormalExport:
    case ModeGetDirectory:
        m_eventLoop.exit(QDialog::Accepted);
        accept();
        break;
    case ModeExportVideo:
        accept();
        break;
    case ModeNormalExportWithAnimate:
    case ModeExportVideoWithAnimate:
        m_eventLoop.exit(QDialog::Accepted);
        break;
    case ModeExportAnpr:
        if (ui->checkBox_plateList->isChecked() || ui->checkBox_video->isChecked() || ui->checkBox_picture->isChecked()) {
            accept();
        } else {
            ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        }
        break;
    case ModeExportPos:
        if (ui->checkBoxExportPosList->isChecked() || ui->checkBoxExportPosVideo->isChecked()) {
            accept();
        } else {
            ShowMessageBox(GET_TEXT("COMMONBACKUP/100018", "Please select at least one file."));
        }
        break;
    default:
        accept();
        break;
    }

    m_exportPos = ui->pushButton_export->mapToGlobal(ui->pushButton_export->rect().center());
}

void MyFileSystemDialog::on_listWidget_device_itemClicked(QListWidgetItem *item)
{
    const DeviceInfoWidget *deviceWidget = qobject_cast<DeviceInfoWidget *>(ui->listWidget_device->itemWidget(item));
    m_currentDevice = deviceWidget;
    if (deviceWidget) {
        ui->label_path->setText(deviceWidget->devicePath());
        if (deviceWidget->isProtected()) {
            ui->tableView_file->setErrorString(GET_TEXT("COMMONBACKUP/100052", "The USB disk is read-only. Please confirm and try again."));
            return;
        }
        if (!deviceWidget->isFormated()) {
            ui->tableView_file->setErrorString(GET_TEXT("COMMONBACKUP/100030", "The current format does not support, please format the device."));
            return;
        }
        const QString &strPath = deviceWidget->devicePath();
        ui->tableView_file->setRootIndexPath(strPath);
    }
}

void MyFileSystemDialog::on_pushButton_newFolder_clicked()
{
    if (!m_currentDevice) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100035", "The current external device does not exist or is damaged, check the external device and refresh the list."));
        return;
    }
    if (!m_newFolder) {
        m_newFolder = new NewFolderDialog(this);
        connect(m_newFolder, SIGNAL(sig_newFolder(QString)), this, SLOT(onNewFolder(QString)));
    }
    m_newFolder->show();
}

void MyFileSystemDialog::on_pushButton_format_clicked()
{
    if (!m_currentDevice) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100035", "The current external device does not exist or is damaged, check the external device and refresh the list."));
        return;
    }
    if (m_currentDevice->isProtected()) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100052", "The USB disk is read-only. Please confirm and try again."));
        return;
    }

    if (!m_formatDialog) {
        m_formatDialog = new FormatDialog(this);
    }
    m_formatDialog->formatDevice(m_currentDevice->usbInfo());
    int result = m_formatDialog->exec();
    if (result == FormatDialog::Accepted) {
        on_pushButton_refresh_clicked();
    }
}

void MyFileSystemDialog::on_pushButton_refresh_clicked()
{
    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_watting->//showWait();
}
