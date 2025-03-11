#include "FormatDialog.h"
#include "ui_FormatDialog.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include <QtDebug>

FormatDialog::FormatDialog(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::FormatDialog)
{
    ui->setupUi(this);

    ui->comboBox_fileSystem->clear();
    ui->comboBox_fileSystem->addItem(QString("FAT32"), FORMAT_FAT32_TYPE);
    ui->comboBox_fileSystem->addItem(QString("NTFS"), FORMAT_NTFS_TYPE);
    onLanguageChanged();
}

FormatDialog::~FormatDialog()
{
    delete ui;
}

void FormatDialog::formatDevice(const resp_usb_info &usb_info)
{
    memcpy(&m_usb_info, &usb_info, sizeof(resp_usb_info));

    ui->lineEdit_name->setText(QString(usb_info.dev_name));
    ui->progressBar->setValue(0);
}

void FormatDialog::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    if (!isVisible()) {
        return;
    }

    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE_INIT:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE_INIT(message);
        message->accept();
        break;
    }
}

void FormatDialog::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_FORMAT_EXPORT_DISK:
        ON_RESPONSE_FLAG_FORMAT_EXPORT_DISK(message);
        break;
    }
}

void FormatDialog::ON_RESPONSE_FLAG_FORMAT_EXPORT_DISK(MessageReceive *message)
{
    int result = -1;
    if (message->data) {
        result = (*(int *)(message->data));
    }
    if (result == -1) {
        ShowMessageBox(GET_TEXT("DISKMANAGE/72016", "Initialized failed."));
    }
    ui->lineEdit_name->setEnabled(true);
    ui->pushButton_cancel->setEnabled(true);
    ui->pushButton_format->setEnabled(true);
    ui->comboBox_fileSystem->setEnabled(true);
    accept();
}

void FormatDialog::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE_INIT(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "FormatDialog::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE_INIT, data is null.";
        return;
    }
    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    ui->progressBar->setValue(progressinfo->percent);
}

void FormatDialog::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("DISKMANAGE/72051", "Format"));
    ui->labelDeviceName->setText(GET_TEXT("SYSTEMGENERAL/70009", "Device Name"));
    ui->labelFileSystem->setText(GET_TEXT("SYSTEMGENERAL/70043", "File System"));
    ui->labelProgress->setText(GET_TEXT("SYSTEMGENERAL/70071", "Progress"));

    ui->pushButton_format->setText(GET_TEXT("DISKMANAGE/72051", "Format"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void FormatDialog::on_pushButton_format_clicked()
{
    const int &result = MessageBox::question(this, GET_TEXT("COMMONBACKUP/100036", "Formatting will delete all the data on the device, continue?"));
    if (result == MessageBox::Yes) {
        struct req_usb_info usb_info;
        memset(&usb_info, 0, sizeof(req_usb_info));
        snprintf(usb_info.dev_path, sizeof(usb_info.dev_path), "%s", m_usb_info.dev_path);
        snprintf(usb_info.dev_name, sizeof(usb_info.dev_name), "%s", m_usb_info.dev_name);
        usb_info.formatType = ui->comboBox_fileSystem->currentData().toInt();
        sendMessage(REQUEST_FLAG_FORMAT_EXPORT_DISK, (void *)&usb_info, sizeof(struct req_usb_info));

        ui->lineEdit_name->setEnabled(false);
        ui->pushButton_cancel->setEnabled(false);
        ui->pushButton_format->setEnabled(false);
        ui->comboBox_fileSystem->setEnabled(false);
    }
}

void FormatDialog::on_pushButton_cancel_clicked()
{
    reject();
}
