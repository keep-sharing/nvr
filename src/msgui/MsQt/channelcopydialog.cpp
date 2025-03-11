#include "channelcopydialog.h"
#include "ui_channelcopydialog.h"
#include "MsDevice.h"
#include "MsLanguage.h"

ChannelCopyDialog::ChannelCopyDialog(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::ChannelCopyDialog)
{
    ui->setupUi(this);
    setTitleWidget(ui->label_title);

    ui->checkBoxGroup->setCount(qMsNvr->maxChannel());
    onLanguageChanged();
}

ChannelCopyDialog::~ChannelCopyDialog()
{
    delete ui;
}

void ChannelCopyDialog::setTitle(const QString &title)
{
    ui->label_title->setText(title);
}

void ChannelCopyDialog::setCount(int count)
{
    ui->checkBoxGroup->setCount(count);
}

void ChannelCopyDialog::setCurrentChannel(int channel)
{
    ui->checkBoxGroup->setIndexEnabled(channel, false);
}

QList<int> ChannelCopyDialog::checkedList(bool containsCurrent/* = true*/)
{
    return ui->checkBoxGroup->checkedList(containsCurrent);
}

quint64 ChannelCopyDialog::checkedFlags(bool containsCurrent/* = true*/) const
{
    return ui->checkBoxGroup->checkedFlags(containsCurrent);
}

void ChannelCopyDialog::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("IMAGE/37341", "Channel Copy"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void ChannelCopyDialog::on_pushButton_ok_clicked()
{
    accept();
}

void ChannelCopyDialog::on_pushButton_cancel_clicked()
{
    reject();
}
