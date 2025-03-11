#include "OccupancyGroupEdit.h"
#include "ui_OccupancyGroupEdit.h"
#include "MsLanguage.h"
#include "MsDevice.h"
#include "MyDebug.h"

OccupancyGroupEdit::OccupancyGroupEdit(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::OccupancyGroupEdit)
{
    ui->setupUi(this);

    ui->checkBoxGroup->setCount(qMsNvr->maxChannel());
    ui->lineEditName->setCheckMode(MyLineEdit::EmptyCheck);

    onLanguageChanged();
}

OccupancyGroupEdit::~OccupancyGroupEdit()
{
    delete ui;
}

int OccupancyGroupEdit::execAdd(PEOPLECNT_SETTING *settings, int index)
{
    m_mode = MODE_ADD;

    ui->label_title->setText(GET_TEXT("OCCUPANCY/74249", "Group Add"));
    m_settings = settings;

    ui->lineEditGroup->setText(QString("%1").arg(index + 1));
    QString groupName(m_settings->name);
    if (groupName.isEmpty())
    {
        groupName = QString("Group %1").arg(index + 1);
    }
    ui->lineEditName->setText(groupName);
    ui->checkBoxGroup->setAllChecked();

    return exec();
}

int OccupancyGroupEdit::execEdit(PEOPLECNT_SETTING *settings, int index)
{
    m_mode = MODE_EDIT;

    ui->label_title->setText(GET_TEXT("OCCUPANCY/74250", "Group Edit"));
    m_settings = settings;

    ui->lineEditGroup->setText(QString("%1").arg(index + 1));
    QString groupName(m_settings->name);
    if (groupName.isEmpty())
    {
        groupName = QString("Group %1").arg(index + 1);
    }
    ui->lineEditName->setText(groupName);
    ui->checkBoxGroup->setCheckedFromString(m_settings->tri_channels);

    return exec();
}

void OccupancyGroupEdit::onLanguageChanged()
{
    ui->labelGroup->setText(GET_TEXT("OCCUPANCY/74251", "Group No."));
    ui->labelName->setText(GET_TEXT("OCCUPANCY/74217", "Group Name"));
    ui->labelChannel->setText(GET_TEXT("ACTION/56007", "Channel"));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void OccupancyGroupEdit::on_pushButtonOk_clicked()
{
    ui->pushButtonOk->clearUnderMouse();

    QString name = ui->lineEditName->text();
    if (!ui->lineEditName->checkValid()) {
        return;
    }
    QList<int> channelList = ui->checkBoxGroup->checkedList();
    if (channelList.isEmpty())
    {
        ShowMessageBox(GET_TEXT("OCCUPANCY/74238", "Please select at least one channel."));
        return;
    }
    m_settings->groupid = ui->lineEditGroup->text().toInt() - 1;
    snprintf(m_settings->name, sizeof(m_settings->name), "%s", name.toStdString().c_str());
    snprintf(m_settings->tri_channels, sizeof(m_settings->tri_channels), "%s", ui->checkBoxGroup->checkedMask().toStdString().c_str());

    accept();
}

void OccupancyGroupEdit::on_pushButtonCancel_clicked()
{
    reject();
}
