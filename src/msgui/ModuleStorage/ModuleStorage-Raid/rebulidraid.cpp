#include "rebulidraid.h"
#include "ui_rebulidraid.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include <QtDebug>

RebulidRaid::RebulidRaid(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::RebulidRaid)
{
    ui->setupUi(this);

    m_waitting = new MsWaitting(this);

    QRegExp rx(QString("[0-9a-zA-Z_-]*"));
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit_name->setValidator(validator);
    ui->lineEdit_name->setMaxLength(32);
    ui->lineEdit_name->setEnabled(false);

    for (int i = 1; i < 17; ++i) {
        QCheckBox *checkBox = ui->widget_port->findChild<QCheckBox *>(QString("checkBox_%1").arg(i));
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxClicked()));
        m_checkBoxList.append(checkBox);
    }
    onLanguageChanged();
}

RebulidRaid::~RebulidRaid()
{
    delete ui;
}

void RebulidRaid::setDiskInfoMap(const QMap<int, BaseDiskInfo> &diskInfoMap)
{
    for (auto iter : m_checkBoxList){
        iter->setChecked(false);
    }
    m_diskInfoMap = diskInfoMap;

    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        int port = i + 1;
        QCheckBox *checkBox = m_checkBoxList.at(i);

        if (m_diskInfoMap.contains(port)) {
            checkBox->setVisible(true);
        } else {
            checkBox->setVisible(false);
        }
        checkBox->setEnabled(diskInfoMap.value(port).isCheckableInRaidWidget());
    }
}

int RebulidRaid::execRaidRebulid(const BaseDiskInfo &raidInfo)
{
    m_rebulidDiskInfo = raidInfo;
    ui->lineEdit_name->setText(raidInfo.vendorString());

    return exec();
}

void RebulidRaid::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_COMPONENT_SIZE_RAID:
        ON_RESPONSE_FLAG_GET_COMPONENT_SIZE_RAID(message);
        break;
    }
}

void RebulidRaid::ON_RESPONSE_FLAG_GET_COMPONENT_SIZE_RAID(MessageReceive *message)
{
    //m_waitting->//closeWait();
    m_rebulidMinSize = *(unsigned long long *)message->data;
}

void RebulidRaid::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("RAID/93014", "RAID Rebuild"));
    ui->label_name->setText(GET_TEXT("DISKMANAGE/72111", "RAID Name"));
    ui->label_port->setText(GET_TEXT("DISKMANAGE/72072", "HDD Port"));
}

void RebulidRaid::onCheckBoxClicked()
{
    QCheckBox *senderCheckBox = (QCheckBox *)sender();
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        QCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox != senderCheckBox) {
            checkBox->setChecked(false);
        }
    }
}

void RebulidRaid::on_pushButton_rebulid_clicked()
{
    const QString &strName = ui->lineEdit_name->text().trimmed();
    if (strName.isEmpty()) {
        ShowMessageBox(GET_TEXT("DISKMANAGE/72110", "The name cannot be empty!"));
        return;
    }

    int checkedPort = -1;
    BaseDiskInfo checkedDiskInfo;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        int port = i + 1;
        QCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            checkedPort = port;
            checkedDiskInfo = m_diskInfoMap.value(port);
        }
    }

    int raidPort = m_rebulidDiskInfo.port();
    sendMessage(REQUEST_FLAG_GET_COMPONENT_SIZE_RAID, (void *)&raidPort, sizeof(int));
    m_waitting->execWait();
    if (checkedDiskInfo.totalBytes() <= m_rebulidMinSize) {
        ShowMessageBox(GET_TEXT("RAID/93015", "Please use a hard disk with capacity larger than the original one to rebuild the array."));
        return;
    }

    struct raid_op_t raidinfo;
    memset(&raidinfo, 0, sizeof(struct raid_op_t));
    snprintf(raidinfo.raid_vendor, sizeof(raidinfo.raid_vendor), "%s", strName.toStdString().c_str());
    raidinfo.raid_port = raidPort;
    raidinfo.raid_level = m_rebulidDiskInfo.raidLevel();
    raidinfo.disk_port[0] = checkedPort;
    raidinfo.disk_num = 1;
    qDebug() << QString("CreateRaid::on_pushButton_rebulid_clicked, REQUEST_FLAG_REBUILD_RAID, port: %1, name: %2").arg(raidPort).arg(strName);
    sendMessage(REQUEST_FLAG_REBUILD_RAID, (void *)&raidinfo, sizeof(struct raid_op_t));

    accept();
}

void RebulidRaid::on_pushButton_cancel_clicked()
{
    reject();
}
