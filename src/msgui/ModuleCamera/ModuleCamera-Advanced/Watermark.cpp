#include "Watermark.h"
#include "ui_Watermark.h"
#include "DrawSceneMotion.h"
#include "DrawView.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "watermark_p.h"

Watermark::Watermark(QWidget *parent)
    : AbstractAdvancedSettingsPage(parent)
    , ui(new Ui::Watermark)
{
    ui->setupUi(this);
    d = new WatermarkPrivate(this);
    d->initializeUI();
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), d, SLOT(onLanguageChanged()));
    d->onLanguageChanged();
    connect(d, SIGNAL(isSupportedChanged()), d, SLOT(onIsSupportedChanged()));
    d->setIsSupported(false);

    ui->lineEdit_watermarkString->setCheckMode(MyLineEdit::EmptyCheck);
}

Watermark::~Watermark()
{
    delete ui;
}

void Watermark::initializeData()
{
    d->resetUI();

    ui->lineEdit_watermarkString->setValid(true);
    ui->pushButton_copy->setEnabled(false);
    ui->pushButton_apply->setEnabled(false);

    ui->widgetMessage->hide();

    if (!isChannelConnected()) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }

    char version[64];
    //get_channel_version_name(channel(), version, sizeof(version));
    MsCameraVersion cameraVersion(version);

    d->setIsSupported(false);
    if (cameraVersion >= MsCameraVersion(7, 74)) {
        d->setIsSupported(true);
    }

    if (!d->isSupported()) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }

    ui->pushButton_copy->setEnabled(true);
    ui->pushButton_apply->setEnabled(true);

    int c = channel();
    Q_UNUSED(c)
}

void Watermark::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_WATERMARK:
        d->ON_RESPONSE_FLAG_GET_IPC_WATERMARK(message->data);
        break;
    case RESPONSE_FLAG_SET_IPC_WATERMARK:
        d->ON_RESPONSE_FLAG_SET_IPC_WATERMARK(message->data);
        break;
    default:
        break;
    }
}

bool Watermark::isInputValid()
{
    bool valid = ui->lineEdit_watermarkString->checkValid();
    return valid;
}

void Watermark::on_pushButton_apply_clicked()
{
    ////showWait();
    ms_water_mark info;
    d->fetchData(&info);
    if (info.enable && !isInputValid()) {
        //closeWait();
        return;
    }
    sendMessage(REQUEST_FLAG_SET_IPC_WATERMARK, &info, sizeof(ms_water_mark));
}

void Watermark::on_pushButton_back_clicked()
{
    back();
}

void Watermark::on_pushButton_copy_clicked()
{
    ChannelCopyDialog channelCopyDialog(this);
    channelCopyDialog.setCurrentChannel(channel());
    int result = channelCopyDialog.exec();
    if (result != QDialog::Accepted)
        return;
    ms_water_mark info;
    d->fetchData(&info);
    if (info.enable && !isInputValid()) {
        //closeWait();
        return;
    }
    QList<int> copyList = channelCopyDialog.checkedList();
    for (int i = 0; i < copyList.size(); ++i) {
        ////showWait();
        info.chanid = copyList.at(i);
        sendMessage(REQUEST_FLAG_SET_IPC_WATERMARK, &info, sizeof(ms_water_mark));
    }
}

void Watermark::on_comboBox_watermark_currentIndexChanged(int index)
{
    bool enable = ui->comboBox_watermark->itemData(index).toBool();
    ui->lineEdit_watermarkString->setEnabled(enable);
}

WatermarkPrivate::WatermarkPrivate(QObject *parent)
    : QObject(parent)
{
    q = qobject_cast<Watermark *>(parent);
}

void WatermarkPrivate::initializeUI()
{
    q->ui->comboBox_watermark->addItem(GET_TEXT("COMMON/1009", "Enable"), true);
    q->ui->comboBox_watermark->addItem(GET_TEXT("COMMON/1018", "Diable"), false);
    q->ui->lineEdit_watermarkString->setMaxLength(16);
}

void WatermarkPrivate::resetUI()
{
    q->ui->comboBox_watermark->setCurrentIndex(1);
    q->ui->comboBox_watermark->setEnabled(false);
    q->ui->lineEdit_watermarkString->setText("IP CAMERA");
    q->ui->lineEdit_watermarkString->setEnabled(false);
}

void WatermarkPrivate::onLanguageChanged()
{
    for (int i = 0; i < q->ui->comboBox_watermark->count(); i++) {
        bool enabled = q->ui->comboBox_watermark->itemData(i).toBool();
        q->ui->comboBox_watermark->setItemText(i, enabled ? GET_TEXT("COMMON/1009", "Enable") : GET_TEXT("COMMON/1018", "Diable"));
    }
    q->ui->label_watermark->setText(GET_TEXT("CAMERAADVANCE/106001", "Watermark"));
    q->ui->label_waterMarkString->setText(GET_TEXT("CAMERAADVANCE/106002", "Watermark String"));
    q->ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    q->ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    q->ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void WatermarkPrivate::ON_RESPONSE_FLAG_GET_IPC_WATERMARK(void *data)
{
    //q->//closeWait();
    if (Q_UNLIKELY(!data)) {
        qWarning() << __FUNCTION__ << ": data is null!";
        return;
    }

    ms_water_mark *info = reinterpret_cast<ms_water_mark *>(data);
    if (qMsNvr->isSigma(info->chanid) && !info->watermarkSupport) {
        setIsSupported(false);
        q->ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        q->ui->pushButton_copy->setEnabled(false);
        q->ui->pushButton_apply->setEnabled(false);
        return;
    }
    q->ui->comboBox_watermark->setCurrentIndex(info->enable ? 0 : 1);
    q->ui->lineEdit_watermarkString->setText(info->pWaterMark);
}

void WatermarkPrivate::ON_RESPONSE_FLAG_SET_IPC_WATERMARK(void *)
{
    //q->//closeWait();
}

bool WatermarkPrivate::isSupported()
{
    return m_isSupported;
}

void WatermarkPrivate::setIsSupported(bool b)
{
    if (m_isSupported == b)
        return;
    m_isSupported = b;
    emit isSupportedChanged();
}

void WatermarkPrivate::onIsSupportedChanged()
{
    q->ui->comboBox_watermark->setEnabled(isSupported());
    bool enable = q->ui->comboBox_watermark->itemData(q->ui->comboBox_watermark->currentIndex()).toBool();
    q->ui->lineEdit_watermarkString->setEnabled(enable);
    q->ui->pushButton_copy->setEnabled(isSupported());
    q->ui->pushButton_apply->setEnabled(isSupported());
}

void WatermarkPrivate::fetchData(ms_water_mark *info)
{
    memset(info, '\0', sizeof(ms_water_mark));
    info->chanid = q->channel();
    info->enable = q->ui->comboBox_watermark->currentIndex() ? 0 : 1;
    QByteArray bytes = q->ui->lineEdit_watermarkString->text().toLocal8Bit();
    memcpy(info->pWaterMark, bytes.data(), qMin(MAX_LEN_32 - 1, bytes.size()));
}
