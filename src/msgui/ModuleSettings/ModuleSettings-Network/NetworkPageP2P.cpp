#include "NetworkPageP2P.h"
#include "ui_NetworkPageP2P.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "centralmessage.h"
#include <QtDebug>

NetworkPageP2P::NetworkPageP2P(QWidget *parent)
    : AbstractNetworkPage(parent)
    , ui(new Ui::NetworkPageP2P)
{
    ui->setupUi(this);

    ui->comboBox_p2p->clear();
    ui->comboBox_p2p->addTranslatableItem("COMMON/1018", "Disable", false);
    ui->comboBox_p2p->addTranslatableItem("COMMON/1009", "Enable", true);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    ui->comboBoxRegion->clear();
    ui->comboBoxRegion->addTranslatableItem("COMMON/1006", "All", TUTK_REGION_ALL);
    ui->comboBoxRegion->addTranslatableItem("P2P/163000", "Africa", TUTK_REGION_AFR);
    ui->comboBoxRegion->addTranslatableItem("P2P/163001", "America", TUTK_REGION_AME);
    ui->comboBoxRegion->addTranslatableItem("P2P/163002", "Asia", TUTK_REGION_ASIA);
    ui->comboBoxRegion->addTranslatableItem("P2P/163003", "Europe", TUTK_REGION_EU);
    ui->comboBoxRegion->addTranslatableItem("P2P/163004", "Middle East", TUTK_REGION_ME);
    ui->comboBoxRegion->addTranslatableItem("P2P/163005", "Oceania", TUTK_REGION_OCEA);

    onLanguageChanged();
}

NetworkPageP2P::~NetworkPageP2P()
{
    delete ui;
}

void NetworkPageP2P::initializeData()
{
    memset(&m_dbP2P, 0, sizeof(p2p_info));
    read_p2p_info(SQLITE_FILE_NAME, &m_dbP2P);

    const QStringList &macList = qMsNvr->macList();

    ui->comboBox_p2p->setCurrentIndexFromData(m_dbP2P.enable);
    on_comboBox_p2p_activated(ui->comboBox_p2p->currentIndex());
    if (!macList.isEmpty()) {
        ui->widget_code->setText(QString("%1&devicetype=1").arg(macList.first()));
    }
    if (m_dbP2P.enable) {
        sendMessage(REQUEST_FLAG_P2P_STATUS_INFO, nullptr, 0);
    } else {
        ui->lineEdit_p2p_status->setText(GET_TEXT("SYSTEMNETWORK/71140", "Unactivated"));
    }
}

void NetworkPageP2P::processMessage(MessageReceive *message)
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

void NetworkPageP2P::ON_RESPONSE_FLAG_P2P_STATUS_INFO(MessageReceive *message)
{
    struct resp_p2p_info *p2p_info = static_cast<struct resp_p2p_info *>(message->data);
    if (!p2p_info) {
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
    ui->comboBoxRegion->setCurrentIndex(p2p_info->region);
}

void NetworkPageP2P::ON_RESPONSE_FLAG_ENABLE_P2P(MessageReceive *message)
{
    //closeWait();
    if (!message->data) {
        qWarning() << "NetworkPageP2P::ON_RESPONSE_FLAG_ENABLE_P2P, data is null.";
        return;
    }
    int result = *((int *)message->data);
    if (result == ERR_OK) {
        write_p2p_info(SQLITE_FILE_NAME, &m_dbP2P);
        ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71112", "Save successfully!"));
        sendMessage(REQUEST_FLAG_P2P_STATUS_INFO, nullptr, 0);
    } else {
        ShowMessageBox("Save Failed!");
    }
}

void NetworkPageP2P::onLanguageChanged()
{
    ui->comboBox_p2p->retranslate();
    ui->comboBoxRegion->retranslate();
    ui->label_note->setText(GET_TEXT("SYSTEMNETWORK/71046", "Enable access to NVR via mobile phone without router setting. Before using this feature, please make sure your NVR is available for the Internet."));
    ui->label_p2p->setText(GET_TEXT("SYSTEMNETWORK/71130", "P2P Service"));
    ui->label_p2p_status->setText(GET_TEXT("SYSTEMNETWORK/71131", "P2P Status"));
    ui->label_note_2->setText(GET_TEXT("SYSTEMNETWORK/71053", "Please scan this QR code on App to get a remote view."));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->labelRegion->setText(GET_TEXT("PTZCONFIG/36056", "Region"));
}

void NetworkPageP2P::onTimeout()
{
    sendMessage(REQUEST_FLAG_P2P_STATUS_INFO, nullptr, 0);
}

void NetworkPageP2P::on_comboBox_p2p_activated(int index)
{
    bool enable = ui->comboBox_p2p->itemData(index).toBool();
    ui->label_p2p_status->setVisible(enable);
    ui->lineEdit_p2p_status->setVisible(enable);
    ui->label_note_2->setVisible(enable);
    ui->widget_code->setVisible(enable);
    ui->comboBoxRegion->setVisible(enable);
    ui->labelRegion->setVisible(enable);
}

void NetworkPageP2P::on_pushButton_apply_clicked()
{
    bool enable = ui->comboBox_p2p->currentData().toBool();
    m_dbP2P.enable = enable;
    m_dbP2P.region = static_cast<TutkRegion>(ui->comboBoxRegion->currentIndex());
    if (enable) {
        sendMessage(REQUEST_FLAG_ENABLE_P2P, &m_dbP2P, sizeof(p2p_info));
        //m_timer->start(60 * 1000);
        //showWait();
    } else {
        write_p2p_info(SQLITE_FILE_NAME, &m_dbP2P);
        sendMessage(REQUEST_FLAG_DISABLE_P2P, &m_dbP2P, sizeof(p2p_info));
        //m_timer->stop();
    }
}

void NetworkPageP2P::on_pushButton_back_clicked()
{
    emit sig_back();
}
