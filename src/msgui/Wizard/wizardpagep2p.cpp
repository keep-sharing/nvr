#include "wizardpagep2p.h"
#include "ui_wizardpagep2p.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include <QtDebug>

WizardPageP2P::WizardPageP2P(QWidget *parent)
    : AbstractWizardPage(parent)
    , ui(new Ui::WizardPageP2P)
{
    ui->setupUi(this);
    ui->comboBox_p2p_service->clear();
    ui->comboBox_p2p_service->addItem("Disable", 0);
    ui->comboBox_p2p_service->addItem("Enable", 1);

    ui->widget_code->setBorderColor(QColor("#FFFFFF"));
    ui->widget_code->setDataColor(Qt::transparent);
}

WizardPageP2P::~WizardPageP2P()
{
    delete ui;
}

void WizardPageP2P::initializeData()
{
    onLanguageChanged();
    memset(&m_dbP2P, 0, sizeof(p2p_info));
    read_p2p_info(SQLITE_FILE_NAME, &m_dbP2P);

    const QStringList &macList = qMsNvr->macList();
    if (!macList.isEmpty()) {
        ui->widget_code->setText(QString("%1&devicetype=1").arg(macList.first()));
    }
    ui->comboBox_p2p_service->setCurrentIndexFromData(m_dbP2P.enable);
    set_P2P_Visible(m_dbP2P.enable);
    sendMessage(REQUEST_FLAG_P2P_STATUS_INFO, nullptr, 0);
}

void WizardPageP2P::set_P2P_Visible(bool enable)
{
    ui->label_p2p_status->setVisible(enable);
    ui->lineEdit_p2p_status->setVisible(enable);
    ui->label_p2p_note->setVisible(enable);
    ui->widget_code->setVisible(enable);
}

void WizardPageP2P::on_comboBox_p2p_service_activated(int index)
{
    bool enable = ui->comboBox_p2p_service->itemData(index).toBool();
    set_P2P_Visible(enable);

    m_dbP2P.enable = enable;
    if (enable) {
        ui->lineEdit_p2p_status->setText("");
        sendMessage(REQUEST_FLAG_ENABLE_P2P, &m_dbP2P, sizeof(m_dbP2P));
        //m_waitting->//showWait();
    } else {
        write_p2p_info(SQLITE_FILE_NAME, &m_dbP2P);
        sendMessage(REQUEST_FLAG_DISABLE_P2P, nullptr, 0);
    }
}

void WizardPageP2P::saveSetting()
{
    //qDebug()<<QString("void WizardPageP2P::saveSetting()");
}

void WizardPageP2P::previousPage()
{
    showWizardPage(Wizard_Camera);
}

void WizardPageP2P::nextPage()
{
    showWizardPage(Wizard_Record);
}

void WizardPageP2P::skipWizard()
{
    AbstractWizardPage::skipWizard();
}

void WizardPageP2P::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_P2P_STATUS_INFO:
        ON_RESPONSE_FLAG_P2P_STATUS_INFO(message);
        break;
    case RESPONSE_FLAG_ENABLE_P2P:
        ON_RESPONSE_FLAG_ENABLE_P2P(message);
        break;
    }
}

void WizardPageP2P::ON_RESPONSE_FLAG_P2P_STATUS_INFO(MessageReceive *message)
{
    struct resp_p2p_info *p2p_info = static_cast<struct resp_p2p_info *>(message->data);
    if (!p2p_info) {
        ui->lineEdit_p2p_status->setText(GET_TEXT("SYSTEMNETWORK/71141", "Unknown error"));
        qWarning() << "NetworkPageP2P::ON_RESPONSE_FLAG_P2P_STATUS_INFO, data is null.";
        return;
    }

    QString stateString;
    switch (p2p_info->p2p_status) {
    case ERR_OK:
        stateString = GET_TEXT("SYSTEMNETWORK/71132", "Activated");
        break;
    case ERR_UNAUTH:
        stateString = GET_TEXT("SYSTEMNETWORK/71133", "P2P request failed");
        break;
    case ERR_NOTFOUND:
        stateString = GET_TEXT("SYSTEMNETWORK/71134", "Network unavailable");
        break;
    case ERR_P2P_NETWORK:
        stateString = GET_TEXT("SYSTEMNETWORK/71134", "Network unavailable");
        break;
    case ERR_AUTH_FAILED:
        stateString = GET_TEXT("SYSTEMNETWORK/71136", "Authentication request failed");
        break;
    case ERR_NO_UUID:
        stateString = GET_TEXT("SYSTEMNETWORK/71137", "No idle UUID");
        break;
    case ERR_P2P_INIT_FAILED:
        stateString = GET_TEXT("SYSTEMNETWORK/71138", "Initialization failed");
        break;
    case ERR_P2P_LOGIN_FAILED:
        stateString = GET_TEXT("SYSTEMNETWORK/71139", "Login failed");
        break;
    case ERR_UNKNOWN:
        stateString = GET_TEXT("SYSTEMNETWORK/71141", "Unknown error");
        break;
    default:
        stateString = GET_TEXT("SYSTEMNETWORK/71141", "Unknown error");
    }
    ui->lineEdit_p2p_status->setText(stateString);
}

void WizardPageP2P::ON_RESPONSE_FLAG_ENABLE_P2P(MessageReceive *message)
{
    //m_waitting->//closeWait();
    if (!message->data) {
        qWarning() << "NetworkPageP2P::ON_RESPONSE_FLAG_ENABLE_P2P, data is null.";
        return;
    }
    int result = *((int *)message->data);
    if (result == ERR_OK) {
        write_p2p_info(SQLITE_FILE_NAME, &m_dbP2P);
        //ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71112", "Save successfully!"));
        sendMessage(REQUEST_FLAG_P2P_STATUS_INFO, nullptr, 0);
    } else {
        ShowMessageBox("Save Failed!");
        ui->lineEdit_p2p_status->setText(GET_TEXT("SYSTEMNETWORK/71140", "Unactivated"));
        qDebug() << "WizardPageP2P Save Failed!";
    }
}

void WizardPageP2P::onLanguageChanged()
{
    ui->label_p2p_service->setText(GET_TEXT("SYSTEMNETWORK/71130", "P2P Service"));
    ui->label_p2p_status->setText(GET_TEXT("SYSTEMNETWORK/71131", "P2P Status"));
    ui->label_p2p_note->setText(GET_TEXT("SYSTEMNETWORK/71053", "Please scan this QR code on App to get a remote view."));

    ui->comboBox_p2p_service->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_p2p_service->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));
}
