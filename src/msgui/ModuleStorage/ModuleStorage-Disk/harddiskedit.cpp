#include "harddiskedit.h"
#include "ui_harddiskedit.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include <QtDebug>

HardDiskEdit::HardDiskEdit(QWidget *parent)
    : AbstractDiskEdit(parent)
    , ui(new Ui::HardDiskEdit)
{
    ui->setupUi(this);

    ui->comboBox_group->clear();
    for (int i = 0; i < 16; ++i) {
        ui->comboBox_group->addItem(QString::number(i + 1), i);
    }

    ui->comboBox_property->clear();
    ui->comboBox_property->addItem(GET_TEXT("DISKMANAGE/72107", "R/W"), 1);
    ui->comboBox_property->addItem(GET_TEXT("DISKMANAGE/72108", "Read-only"), 0);

    ui->comboBox_private->clear();
    ui->comboBox_private->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_private->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    onLanguageChanged();
}

HardDiskEdit::~HardDiskEdit()
{
    delete ui;
}

void HardDiskEdit::setDiskInfo(const BaseDiskInfo &info)
{
    AbstractDiskEdit::setDiskInfo(info);

    ui->lineEdit_port->setText(QString::number(info.port()));
    ui->lineEdit_capacity->setText(info.totalBytesString());
    ui->comboBox_group->setCurrentIndexFromData(info.groupValue());
    ui->comboBox_group->setEnabled(info.is_group_enable);
    ui->comboBox_property->setCurrentIndexFromData(info.propertyValue());
    ui->comboBox_private->setCurrentIndexFromData(info.privateValue());
}

void HardDiskEdit::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_MSFS_PORT:
        ON_RESPONSE_FLAG_SET_MSFS_PORT(message);
        break;
    }
}

void HardDiskEdit::ON_RESPONSE_FLAG_SET_MSFS_PORT(MessageReceive *message)
{
    Q_UNUSED(message)
    //中心没有返回这个消息
    ////m_waitting->//closeWait();
}

void HardDiskEdit::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("DISK/92037", "Disk Edit"));
    ui->label_port->setText(GET_TEXT("CHANNELMANAGE/30011", "Port"));
    ui->label_capacity->setText(GET_TEXT("RAID/93004", "Capacity"));
    ui->label_group->setText(GET_TEXT("DISK/92004", "Group"));
    ui->label_property->setText(GET_TEXT("DISK/92016", "Property"));
    ui->label_private->setText(GET_TEXT("DISK/92038", "Private"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void HardDiskEdit::on_pushButton_ok_clicked()
{
    int group = ui->comboBox_group->currentData().toInt();
    int property = ui->comboBox_property->currentData().toInt();
    int iPrivate = ui->comboBox_private->currentData().toInt();
    if (group != m_diskInfo.groupValue()) {
        const int result = MessageBox::question(this, GET_TEXT("DISK/92017", "The Group of Disk has been changed, you may need reallocate the channels for the Group."));
        if (result == MessageBox::Cancel) {
            return;
        }
    }
    if (group != m_diskInfo.groupValue() || property != m_diskInfo.propertyValue() || iPrivate != m_diskInfo.privateValue()) {
        if (!property) {
            const int result = MessageBox::question(this, GET_TEXT("DISK/92031", "Recording will stop after changing to Read-Only Mode，continue？"));
            if (result == MessageBox::Cancel) {
                return;
            }
        }

        if (iPrivate == 1) {
            MessageBox::instance()->execInformation(GET_TEXT("DISK/92039", "The data on this disk will only be accessed by this NVR!"));
        }

        m_diskInfo.setGroupValue(group);
        m_diskInfo.setPropertyValue(property);
        m_diskInfo.saveToDatebase();

        struct req_port_info port_info;
        memset(&port_info, 0, sizeof(struct req_port_info));
        port_info.id = m_diskInfo.port();
        port_info.enRw = property;
        port_info.group = group;
        port_info.iPrivate = iPrivate;
        qDebug() << QString("REQUEST_FLAG_SET_MSFS_PORT, id: %1, enRw: %2, group: %3, iPrivate: %4").arg(port_info.id).arg(port_info.enRw).arg(port_info.group).arg(port_info.iPrivate);
        sendMessageOnly(REQUEST_FLAG_SET_MSFS_PORT, (void *)&port_info, sizeof(struct req_port_info));
        //m_waitting->execWait();
        accept();
    } else {
        reject();
    }
}

void HardDiskEdit::on_pushButton_cancel_clicked()
{
    reject();
}
