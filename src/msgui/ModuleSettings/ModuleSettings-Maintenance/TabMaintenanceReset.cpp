#include "TabMaintenanceReset.h"
#include "ui_TabMaintenanceReset.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "msuser.h"

extern "C" {
#include "log.h"
}

TabMaintenanceReset::TabMaintenanceReset(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabMaintenanceReset)
{
    ui->setupUi(this);
    onLanguageChanged();
}

TabMaintenanceReset::~TabMaintenanceReset()
{
    delete ui;
}

void TabMaintenanceReset::initializeData()
{
}

void TabMaintenanceReset::onLanguageChanged()
{
    ui->label_note->setText(GET_TEXT("PROFILE/76015", "Please choose the parameters which you want to keep after reset.Uncheck all to reset the NVR to Inactivate default."));
    ui->checkBox_ip->setText(GET_TEXT("PROFILE/76017", "Keep the IP Configuration"));
    ui->checkBox_user->setText(GET_TEXT("PROFILE/76018", "Keep the User Information"));
    ui->pushButton_reset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_back->setText(GET_TEXT("DISKSTATUS/66008", "Back"));
}

void TabMaintenanceReset::on_pushButton_reset_clicked()
{
    const int &result = MessageBox::question(this, GET_TEXT("PROFILE/76012", "Note: The system will reboot automatically. Continue?"));
    if (result == MessageBox::Yes) {
        struct reset_type reset;
        reset.keepIp = ui->checkBox_ip->isChecked();
        reset.keepUser = ui->checkBox_user->isChecked();
        Q_UNUSED(reset)

        struct log_data log_data;
        memset(&log_data, 0, sizeof(struct log_data));
        log_data.log_data_info.subType = SUB_OP_PROFILE_RESET_LOCAL;
        log_data.log_data_info.parameter_type = SUB_PARAM_NONE;
        snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
        log_data.log_data_info.chan_no = 0;
        MsWriteLog(log_data);

        sendMessageOnly(REQUEST_FLAG_RESET, nullptr, 0);

        //m_waitting->//showWait();
    }
}

void TabMaintenanceReset::on_pushButton_back_clicked()
{
    emit sig_back();
}
