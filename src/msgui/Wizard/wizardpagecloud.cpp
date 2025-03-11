#include "wizardpagecloud.h"
#include "ui_wizardpagecloud.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"

extern "C" {
#include "msdb.h"
}

WizardPageCloud::WizardPageCloud(QWidget *parent)
    : AbstractWizardPage(parent)
    , ui(new Ui::WizardPageCloud)
{
    ui->setupUi(this);

    ui->comboBoxMilesightCloud->clear();
    ui->comboBoxMilesightCloud->addTranslatableItem("COMMON/1018", "Disable", false);
    ui->comboBoxMilesightCloud->addTranslatableItem("COMMON/1009", "Enable", true);

    ui->comboBoxRegion->clear();
    ui->comboBoxRegion->addTranslatableItem("COMMON/1006", "All", TUTK_REGION_ALL);
    ui->comboBoxRegion->addTranslatableItem("P2P/163000", "Africa", TUTK_REGION_AFR);
    ui->comboBoxRegion->addTranslatableItem("P2P/163001", "America", TUTK_REGION_AME);
    ui->comboBoxRegion->addTranslatableItem("P2P/163002", "Asia", TUTK_REGION_ASIA);
    ui->comboBoxRegion->addTranslatableItem("P2P/163003", "Europe", TUTK_REGION_EU);
    ui->comboBoxRegion->addTranslatableItem("P2P/163004", "Middle East", TUTK_REGION_ME);
    ui->comboBoxRegion->addTranslatableItem("P2P/163005", "Oceania", TUTK_REGION_OCEA);

    m_labelStatus = new QLabel(this);
    m_labelStatus->setStyleSheet("border: 1px solid #8196AF; color: #4A4A4A; background-color: #FEFEFF; min-height: 30px;");
    m_labelStatus->hide();

    ui->toolButtonMilesightCloudStatus->installEventFilter(this);
    ui->toolButtonMilesightCloudStatus->setParent(this);

    ui->widgetRegisterQRCode->setFixedSize(150, 150);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

WizardPageCloud::~WizardPageCloud()
{
    delete ui;
}

void WizardPageCloud::initializeData()
{
    ui->toolButtonMilesightCloudStatus->hide();

    sendMessage(REQUEST_FLAG_P2P_STATUS_INFO, nullptr, 0);
    //MsWaitting::showGlobalWait();
}

void WizardPageCloud::saveSetting()
{
}

void WizardPageCloud::previousPage()
{
    showWizardPage(Wizard_Camera);
}

void WizardPageCloud::nextPage()
{
    showWizardPage(Wizard_Record);
}

void WizardPageCloud::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_P2P_STATUS_INFO:
        ON_RESPONSE_FLAG_P2P_STATUS_INFO(message);
        break;
    case RESPONSE_FLAG_P2P_UNBIND_IOT_DEVICE:
        ON_RESPONSE_FLAG_P2P_UNBIND_IOT_DEVICE(message);
        break;
    case RESPONSE_FLAG_ENABLE_P2P:
        ON_RESPONSE_FLAG_ENABLE_P2P(message);
        break;
    case RESPONSE_FLAG_DISABLE_P2P:
        ON_RESPONSE_FLAG_DISABLE_P2P(message);
        break;
    }
}

void WizardPageCloud::setRegionVisible(bool visible)
{
    if (visible) {
        ui->comboBoxRegion->show();
        ui->labelRegion->show();
    } else {
        ui->comboBoxRegion->hide();
        ui->labelRegion->hide();
    }
}

bool WizardPageCloud::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->toolButtonMilesightCloudStatus) {
        switch (event->type()) {
        case QEvent::Enter: {
            ui->toolButtonMilesightCloudStatus->setIcon(QIcon(":/common/common/note_hover.png"));
            QPoint p = ui->toolButtonMilesightCloudStatus->mapTo(this, QPoint(0, 0));
            m_labelStatus->show();
            m_labelStatus->adjustSize();
            m_labelStatus->move(p.x() - m_labelStatus->height(), p.y() - 35);
            break;
        }
        case QEvent::Leave: {
            ui->toolButtonMilesightCloudStatus->setIcon(QIcon(":/common/common/note.png"));
            m_labelStatus->hide();
            break;
        }
        default:
            break;
        }
    }
    return AbstractWizardPage::eventFilter(obj, event);
}

void WizardPageCloud::ON_RESPONSE_FLAG_P2P_STATUS_INFO(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    ui->widgetRegisterQRCode->setText("12345");

    struct resp_p2p_info *p2p_info = static_cast<struct resp_p2p_info *>(message->data);
    if (!p2p_info) {
        qMsWarning() << "data is null.";
        return;
    }

    QString stateString;
    switch (p2p_info->p2p_status) {
    case ERR_OK:
        stateString = GET_TEXT("SYSTEMNETWORK/71132", "Activated");
        break;
    case ERR_P2P_NETWORK:
        stateString = GET_TEXT("CLOUD/33012", "Failed to connect with internet.");
        break;
    case ERR_AUTH_FAILED:
        stateString = GET_TEXT("CLOUD/33013", "Failed to authenticate P2P encryption.");
        break;
    case ERR_NO_UUID:
        stateString = GET_TEXT("CLOUD/33014", "Please contact Milesight support.");
        break;
    case ERR_P2P_INIT_FAILED:
        stateString = GET_TEXT("CLOUD/33015", "Failed to login P2P server.");
        break;
    case ERR_P2P_LOGIN_FAILED:
        stateString = GET_TEXT("CLOUD/33015", "Failed to login P2P server.");
        break;
    case ERR_UNKNOWN:
        stateString = GET_TEXT("CLOUD/33017", "Other error.");
        break;
    default:
        stateString = GET_TEXT("CLOUD/33017", "Other error.");
        break;
    }

    ui->comboBoxMilesightCloud->setCurrentIndexFromData(p2p_info->enable);
    setRegionVisible(p2p_info->enable);
    if (p2p_info->enable) {
        if (p2p_info->p2p_status == ERR_OK) {
            ui->labelMilesightCloudStatusString->setText(GET_TEXT("CLOUD/33010", "Online"));
        } else {
            ui->labelMilesightCloudStatusString->setText(GET_TEXT("CLOUD/33011", "Disconnected"));
            m_labelStatus->setText(stateString);
            ui->toolButtonMilesightCloudStatus->show();

            QPoint p = ui->labelMilesightCloudStatusString->mapTo(this, QPoint(0, 0));
            p.setX(p.x() + ui->labelMilesightCloudStatusString->width() + 10);
            p.setY(p.y() + ui->labelMilesightCloudStatusString->height() / 2 - ui->toolButtonMilesightCloudStatus->height() / 2);
            ui->toolButtonMilesightCloudStatus->move(p);
        }
    } else {
        ui->labelMilesightCloudStatusString->setText(GET_TEXT("CLOUD/33009", "Offline"));
    }
    QString account(p2p_info->cloudAccount);
    if (account.isEmpty() || !p2p_info->enable) {
        account = QString("--");
        ui->pushButtonUnbindDevice->setEnabled(false);
    } else {
        ui->pushButtonUnbindDevice->setEnabled(true);
    }
    ui->lineEditCloudAccount->setText(account);
    QRegExp rx("registcode=(.+)&devicetype");
    QString code(p2p_info->registerCode);
    if (rx.indexIn(code) != -1) {
        ui->lineEditRegisterCode->setText(rx.cap(1));
    } else {
        ui->lineEditRegisterCode->setText(code);
    }
    ui->widgetRegisterQRCode->setText(p2p_info->registerCode);
    ui->comboBoxRegion->setCurrentIndex(p2p_info->region);
}

void WizardPageCloud::ON_RESPONSE_FLAG_P2P_UNBIND_IOT_DEVICE(MessageReceive *message)
{
    Q_UNUSED(message)

    initializeData();
}

void WizardPageCloud::ON_RESPONSE_FLAG_ENABLE_P2P(MessageReceive *message)
{
    if (!message->data) {
        qMsWarning() << "data is null.";
        ShowMessageBox("Save Failed!");
    } else {
        int result = *((int *)message->data);
        if (result == ERR_OK) {
            ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71112", "Save successfully!"));
        } else {
            qMsWarning() << QString("enable p2p failed: %1").arg(result);
            ShowMessageBox("Save Failed!");
        }
    }

    initializeData();
}

void WizardPageCloud::ON_RESPONSE_FLAG_DISABLE_P2P(MessageReceive *message)
{
    Q_UNUSED(message)

    initializeData();
}

void WizardPageCloud::onLanguageChanged()
{
    ui->labelMilesightCloud->setText("Milesight Cloud");
    ui->comboBoxMilesightCloud->retranslate();
    ui->comboBoxRegion->retranslate();
    ui->labelRegion->setText(GET_TEXT("PTZCONFIG/36056", "Region"));
    ui->labelMilesightCloudStatus->setText(GET_TEXT("CLOUD/33008", "Milesight Cloud Status"));
    ui->labelCloudAccount->setText(GET_TEXT("CLOUD/33001", "Cloud Account"));
    ui->labelUnbindDevice->setText(GET_TEXT("CLOUD/33002", "Unbind Device"));
    ui->pushButtonUnbindDevice->setText(GET_TEXT("CLOUD/33003", "Unbind"));
    ui->labelCloudServerAddress->setText(GET_TEXT("CLOUD/33005", "Cloud Server Address"));
    ui->labelRegisterCode->setText(GET_TEXT("CLOUD/33000", "Register Code"));
    ui->labelRegisterQRCode->setText(GET_TEXT("CLOUD/33006", "Register QR Code"));
    ui->labelNote1->setText(GET_TEXT("CLOUD/33019", "Note: "));
    ui->labelNote2->setText(GET_TEXT("CLOUD/33007", "Enable Milesight Cloud function, the P2P function will be enabled automatically. Scan the QR code to bind this device to Cloud Account."));
}

void WizardPageCloud::on_comboBoxMilesightCloud_activated(int index)
{
    p2p_info dbP2P;
    memset(&dbP2P, 0, sizeof(p2p_info));
    read_p2p_info(SQLITE_FILE_NAME, &dbP2P);

    dbP2P.region = static_cast<TutkRegion>(ui->comboBoxRegion->currentIndex());
    if (ui->comboBoxMilesightCloud->itemData(index).toBool()) {
        setRegionVisible(true);
        dbP2P.enable = 1;
        sendMessage(REQUEST_FLAG_ENABLE_P2P, &dbP2P, sizeof(p2p_info));
    } else {
        dbP2P.enable = 0;
        setRegionVisible(false);
        write_p2p_info(SQLITE_FILE_NAME, &dbP2P);
        sendMessage(REQUEST_FLAG_DISABLE_P2P, &dbP2P, sizeof(p2p_info));
    }

    //MsWaitting::showGlobalWait();
}

void WizardPageCloud::on_pushButtonUnbindDevice_clicked()
{
    ui->pushButtonUnbindDevice->clearUnderMouse();
    MessageBox::Result result = MessageBox::question(this, GET_TEXT("CLOUD/33018", "The device will be deleted from Cloud Account. Sure to unbind the device?"));
    if (result == MessageBox::Cancel) {
        return;
    }

    sendMessage(REQUEST_FLAG_P2P_UNBIND_IOT_DEVICE, nullptr, 0);
    //MsWaitting::showGlobalWait();
}

void WizardPageCloud::on_comboBoxRegion_activated(int index)
{
    Q_UNUSED(index)
    on_comboBoxMilesightCloud_activated(ui->comboBoxMilesightCloud->currentIntData());
}
