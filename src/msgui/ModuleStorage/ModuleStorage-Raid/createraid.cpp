#include "createraid.h"
#include "ui_createraid.h"
#include "EventLoop.h"
#include "MessageBox.h"
#include "MsDisk.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"

CreateRaid::CreateRaid(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::CreateRaid)
{
    ui->setupUi(this);

    m_waitting = new MsWaitting(this);

    QRegExp rx(QString("[0-9a-zA-Z_-]*"));
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit_name->setValidator(validator);
    ui->lineEdit_name->setMaxLength(32);

    ui->comboBox_type->clear();
    ui->comboBox_type->addItem("RAID 0", RAID_0);
    ui->comboBox_type->addItem("RAID 1", RAID_1);
    ui->comboBox_type->addItem("RAID 5", RAID_5);
    ui->comboBox_type->addItem("RAID 6", RAID_6);
    ui->comboBox_type->addItem("RAID 10", RAID_10);

    for (int i = 1; i < 17; ++i) {
        QCheckBox *checkBox = ui->widget_port->findChild<QCheckBox *>(QString("checkBox_%1").arg(i));
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxClicked()));
        m_checkBoxList.append(checkBox);
    }
    onLanguageChanged();
}

CreateRaid::~CreateRaid()
{
    delete ui;
}

void CreateRaid::setDiskInfoMap(const QMap<int, BaseDiskInfo> &diskInfoMap, const QList<int> &checkedList)
{
    ui->comboBox_type->setCurrentIndexFromData(0);
    m_diskInfoMap = diskInfoMap;

    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        int port = i + 1;
        QCheckBox *checkBox = m_checkBoxList.at(i);

        if (checkedList.contains(port)) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }

        if (m_diskInfoMap.contains(port)) {
            checkBox->setVisible(true);
        } else {
            checkBox->setVisible(false);
        }

        checkBox->setEnabled(diskInfoMap.value(port).isCheckableInRaidWidget());
    }

    //
    ui->lineEdit_name->setText("RAID");
    on_comboBox_type_activated(ui->comboBox_type->currentIndex());
}

void CreateRaid::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_CREATE_RAID:
        ON_RESPONSE_FLAG_CREATE_RAID(message);
        break;
    }
}

void CreateRaid::ON_RESPONSE_FLAG_CREATE_RAID(MessageReceive *message)
{
    int result = *(int *)message->data;
    gEventLoopExit(result);
}

quint64 CreateRaid::checkRaid(int level, const QMap<int, int> &portMap) const
{
    quint64 bytes = 0;
    switch (level) {
    case RAID_0: {
        // 2 disks or above
        if (portMap.size() >= 2) {
            for (auto iter = portMap.constBegin(); iter != portMap.constEnd(); ++iter) {
                int port = iter.key();
                const BaseDiskInfo &info = m_diskInfoMap.value(port);
                bytes += info.totalBytes();
            }
        } else {
            bytes = 0;
        }
        break;
    }
    case RAID_1: {
        // 2 disks
        if (portMap.size() == 2) {
            for (auto iter = portMap.constBegin(); iter != portMap.constEnd(); ++iter) {
                int port = iter.key();
                const BaseDiskInfo &info = m_diskInfoMap.value(port);
                if (bytes > info.totalBytes() || bytes == 0) {
                    if (info.totalBytes() > 0) {
                        bytes = info.totalBytes();
                    }
                }
            }
        } else {
            bytes = 0;
        }
        break;
    }
    case RAID_5: {
        // 3 disks or above
        if (portMap.size() >= 3) {
            for (auto iter = portMap.constBegin(); iter != portMap.constEnd(); ++iter) {
                int port = iter.key();
                const BaseDiskInfo &info = m_diskInfoMap.value(port);
                if (bytes > info.totalBytes() || bytes == 0) {
                    if (info.totalBytes() > 0) {
                        bytes = info.totalBytes();
                    }
                }
            }
            bytes = bytes + bytes * (portMap.size() - 2);
        } else {
            bytes = 0;
        }
        break;
    }
    case RAID_6: {
        // 4 disks or above
        if (portMap.size() >= 4) {
            for (auto iter = portMap.constBegin(); iter != portMap.constEnd(); ++iter) {
                int port = iter.key();
                const BaseDiskInfo &info = m_diskInfoMap.value(port);
                if (bytes > info.totalBytes() || bytes == 0) {
                    if (info.totalBytes() > 0) {
                        bytes = info.totalBytes();
                    }
                }
            }
            bytes = bytes + bytes * (portMap.size() - 3);
        } else {
            bytes = 0;
        }
        break;
    }
    case RAID_10: {
        // 4 disks or above && disks%2 == 0
        if (portMap.size() >= 4 && portMap.size() % 2 == 0) {
            for (auto iter = portMap.constBegin(); iter != portMap.constEnd(); ++iter) {
                int port = iter.key();
                const BaseDiskInfo &info = m_diskInfoMap.value(port);
                if (bytes > info.totalBytes() || bytes == 0) {
                    if (info.totalBytes() > 0) {
                        bytes = info.totalBytes();
                    }
                }
            }
            bytes = bytes * (portMap.size() / 2);
        } else {
            bytes = 0;
        }
        break;
    }
    default:
        break;
    }
    return bytes;
}

void CreateRaid::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("RAID/93012", "RAID Create"));
    ui->label_name->setText(GET_TEXT("DISKMANAGE/72111", "RAID Name"));
    ui->label_type->setText(GET_TEXT("DISKMANAGE/72070", "RAID Type"));
    ui->label_port->setText(GET_TEXT("DISKMANAGE/72072", "HDD Port"));
    ui->label_capacity->setText(GET_TEXT("DISK/92013", "RAID Capacity"));
    ui->pushButton_create->setText(GET_TEXT("DISKMANAGE/72074", "Create"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void CreateRaid::onCheckBoxClicked()
{
    on_comboBox_type_activated(ui->comboBox_type->currentIndex());
}

void CreateRaid::on_comboBox_type_activated(int index)
{
    int level = ui->comboBox_type->itemData(index).toInt();

    QMap<int, int> portMap;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        int port = i + 1;
        QCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            const BaseDiskInfo &info = m_diskInfoMap.value(port);
            portMap.insert(info.port(), 0);
        }
    }

    m_bytes = checkRaid(level, portMap);
    ui->lineEdit_capacity->setText(BaseDiskInfo::bytesString(m_bytes));
}

void CreateRaid::on_pushButton_create_clicked()
{
    const QString &strName = ui->lineEdit_name->text().trimmed();
    if (strName.isEmpty()) {
        ShowMessageBox(GET_TEXT("DISKMANAGE/72110", "The name cannot be empty!"));
        return;
    }

    if (m_bytes <= 0) {
        ShowMessageBox(GET_TEXT("DISKMANAGE/72049", "The number of physical disk is wrong."));
        return;
    }

    if (MessageBox::question(this, GET_TEXT("DISKMANAGE/72046", "HDD data will be erased, continue to create?")) == MessageBox::Cancel) {
        return;
    }

    int level = ui->comboBox_type->currentData().toInt();
    switch (level) {
    case RAID_0:
    case RAID_5:
    case RAID_6:
    case RAID_10:
        //RAID0/5/6/10创建的总容量如果超过16TB则会出现无法录像的情况
        //NOTE 2021-09-14 已经支持16TB
#if 0
        if ((m_bytes >> 20) > quint64(16 << 20)) {
            ShowMessageBox(GET_TEXT("DISKMANAGE/72114", "Total RAID capacity must not exceed 16TB! "));
            return;
        }
#endif
        break;
    default:
        break;
    }

    QMap<int, int> portMap;

    struct raid_op_t raidinfo;
    memset(&raidinfo, 0, sizeof(struct raid_op_t));
    snprintf(raidinfo.raid_vendor, sizeof(raidinfo.raid_vendor), "%s", strName.toStdString().c_str());
    raidinfo.raid_port = gMsDisk.validRaidPort();
    raidinfo.raid_level = level;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        int port = i + 1;
        QCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            portMap.insert(port, 0);
            raidinfo.disk_port[raidinfo.disk_num] = port;
            raidinfo.disk_num++;
        }
    }

    //MsWaitting::showGlobalWait(this);
    sendMessage(REQUEST_FLAG_CREATE_RAID, &raidinfo, sizeof(struct raid_op_t));
    int result = gEventLoopExec();
    //MsWaitting::closeGlobalWait();
    if (result == 0) {
        accept();
    } else if (result < 0) {
        MessageBox::information(this, GET_TEXT("RAID/93017", "Failed to create RAID because some physical disks error."));
    } else {
        for (auto iter = portMap.begin(); iter != portMap.end();) {
            int port = iter.key();
            if (((1 << port) & result)) {
                iter = portMap.erase(iter);
            } else {
                ++iter;
            }
        }
        qint64 bytes = checkRaid(level, portMap);
        if (portMap.size() < 3 || bytes == 0) {
            MessageBox::information(this, GET_TEXT("RAID/93017", "Failed to create RAID because some physical disks error."));
        } else {
            //格式化失败后，去掉失败的盘再试一次
            if (MessageBox::question(this, GET_TEXT("DISK/150000", "Some physical disks error, continue？")) == MessageBox::Cancel) {
                accept();
                return;
            }
            memset(raidinfo.disk_port, 0, sizeof(raidinfo.disk_port));
            raidinfo.disk_num = 0;
            for (auto iter = portMap.begin(); iter != portMap.end(); iter++) {
                int port = iter.key();
                raidinfo.disk_port[raidinfo.disk_num] = port;
                raidinfo.disk_num++;
            }
            //MsWaitting::showGlobalWait(this);
            sendMessage(REQUEST_FLAG_CREATE_RAID, &raidinfo, sizeof(struct raid_op_t));
            result = gEventLoopExec();
            //MsWaitting::closeGlobalWait();
            if (result != 0) {
                MessageBox::information(this, GET_TEXT("RAID/93017", "Failed to create RAID because some physical disks error."));
            }
        }
    }
    accept();
}

void CreateRaid::on_pushButton_cancel_clicked()
{
    reject();
}
