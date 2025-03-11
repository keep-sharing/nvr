#include "imagepageosd.h"
#include "ui_imagepageosd.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include <QTimer>
#include <QtDebug>

ImagePageOsd::ImagePageOsd(QWidget *parent)
    : AbstractImagePage(parent)
    , ui(new Ui::ImagePageOsd)
{
    ui->setupUi(this);

    ui->comboBox_stream->clear();
    ui->comboBox_stream->addItem(GET_TEXT("IMAGE/37343", "All Streams"), OSD_ALL_STREAM);
    ui->comboBox_stream->addItem(GET_TEXT("IMAGE/37333", "Primary Stream"), OSD_MAIN_STREAM);
    ui->comboBox_stream->addItem(GET_TEXT("CHANNELMANAGE/30057", "Secondary Stream"), OSD_SUB_STREAM);

    ui->comboBox_titlePosition->clear();
    ui->comboBox_titlePosition->addItem(GET_TEXT("IMAGE/37335", "Top-Left"), 0);
    ui->comboBox_titlePosition->addItem(GET_TEXT("IMAGE/37336", "Top-Right"), 1);
    ui->comboBox_titlePosition->addItem(GET_TEXT("IMAGE/37337", "Bottom-Left"), 2);
    ui->comboBox_titlePosition->addItem(GET_TEXT("IMAGE/37338", "Bottom-Right"), 3);

    ui->comboBox_datePosition->clear();
    ui->comboBox_datePosition->addItem(GET_TEXT("IMAGE/37335", "Top-Left"), 0);
    ui->comboBox_datePosition->addItem(GET_TEXT("IMAGE/37336", "Top-Right"), 1);
    ui->comboBox_datePosition->addItem(GET_TEXT("IMAGE/37337", "Bottom-Left"), 2);
    ui->comboBox_datePosition->addItem(GET_TEXT("IMAGE/37338", "Bottom-Right"), 3);

    ui->comboBox_dateFormat->setItemData(0, 0);
    ui->comboBox_dateFormat->setItemData(1, 1);
    ui->comboBox_dateFormat->setItemData(2, 2);

    ui->comboBoxFontSize->beginEdit();
    ui->comboBoxFontSize->clear();
    ui->comboBoxFontSize->addItem(GET_TEXT("PEOPLECOUNTING_SEARCH/145015", "Smallest"), FONT_SIZE_SMALLEST);
    ui->comboBoxFontSize->addItem(GET_TEXT("POS/130014", "Small"), FONT_SIZE_SMALL);
    ui->comboBoxFontSize->addItem(GET_TEXT("POS/130015", "Medium"), FONT_SIZE_MEDIUM);
    ui->comboBoxFontSize->addItem(GET_TEXT("POS/130016", "Large"), FONT_SIZE_LARGE);
    ui->comboBoxFontSize->addItem(GET_TEXT("PEOPLECOUNTING_SEARCH/145016", "Largest"), FONT_SIZE_LARGEST);
    ui->comboBoxFontSize->addItem(GET_TEXT("SNAPSHOT/95026", "Auto"), FONT_SIZE_AUTO);
    ui->comboBoxFontSize->endEdit();

    ui->lineEdit_videoTitle->setCheckMode(MyLineEdit::UserNameCheck);

    onLanguageChanged();
}

ImagePageOsd::~ImagePageOsd()
{
    delete ui;
}

void ImagePageOsd::initializeData(int channel)
{
    m_currentChannel = channel;

    clearSettings();
    setSettingsEnable(false);

    if (!isChannelConnected()) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }
    if (!qMsNvr->isMsCamera(m_currentChannel)) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }

    setSettingsEnable(true);

    sendMessage(REQUEST_FLAG_OVF_GET_OSD, (void *)&m_currentChannel, sizeof(int));
    //MsWaitting::showGlobalWait();
}

void ImagePageOsd::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_OVF_GET_OSD:
        ON_RESPONSE_FLAG_OVF_GET_OSD(message);
        break;
    case RESPONSE_FLAG_OVF_SET_OSD:
        ON_RESPONSE_FLAG_OVF_SET_OSD(message);
        break;
    default:
        break;
    }
}

void ImagePageOsd::clearSettings()
{
    ui->comboBox_stream->setCurrentIndexFromData(0);
    ui->comboBox_dateFormat->setCurrentIndexFromData(0);
    ui->comboBox_datePosition->setCurrentIndexFromData(0);
    ui->comboBox_titlePosition->setCurrentIndex(0);
    ui->comboBoxFontSize->setCurrentIndexFromData(1);
    ui->checkBox_showTimestamp->setChecked(false);
    ui->checkBox_showTitle->setChecked(false);
    ui->widgetMessage->hideMessage();
}

void ImagePageOsd::setSettingsEnable(bool enable)
{
    ui->widgetContainer->setEnabled(enable);

    ui->pushButton_copy->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
}

void ImagePageOsd::ON_RESPONSE_FLAG_OVF_GET_OSD(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();

    struct req_ovf_osd *ovf_osd = (struct req_ovf_osd *)message->data;
    if (!ovf_osd) {
        qWarning() << "ImagePageOsd::ON_RESPONSE_FLAG_OVF_GET_OSD, data is null.";
        return;
    }
    memset(&m_ovf_osd, 0, sizeof(req_ovf_osd));
    memcpy(&m_ovf_osd, ovf_osd, sizeof(req_ovf_osd));

    switch (ovf_osd->stream_type) {
    case OSD_ALL_STREAM:
    case OSD_MAIN_STREAM:
    case OSD_SUB_STREAM:
    case OSD_THD_STREAM:
        ui->comboBox_stream->setCurrentIndexFromData(ovf_osd->stream_type);
        on_comboBox_stream_activated(ui->comboBox_stream->currentIndex());
        break;
    default:
        ui->comboBox_stream->setCurrentIndex(0);
        on_comboBox_stream_activated(0);
        break;
    }
}

void ImagePageOsd::ON_RESPONSE_FLAG_OVF_SET_OSD(MessageReceive *message)
{
    Q_UNUSED(message)

    if (m_copyChannelList.isEmpty()) {
        //MsWaitting::closeGlobalWait();
    } else {
        sendCopyData();
    }
}

void ImagePageOsd::sendCopyData()
{
    if (m_copyChannelList.isEmpty()) {
        return;
    }
    int channel = m_copyChannelList.takeFirst();
    m_ovf_osd.ch = channel;
    qDebug() << QString("REQUEST_FLAG_OVF_SET_OSD, channel: %1").arg(channel);
    sendMessage(REQUEST_FLAG_OVF_SET_OSD, &m_ovf_osd, sizeof(struct req_ovf_osd));
}

void ImagePageOsd::onLanguageChanged()
{
    ui->label_stream->setText(GET_TEXT("IMAGE/37326", "Video Stream"));
    ui->label_showTitle->setText(GET_TEXT("IMAGE/37328", "Show Video Title"));
    ui->label_showTimestamp->setText(GET_TEXT("IMAGE/37329", "Show Tilestamp"));
    ui->label_videoTtitle->setText(GET_TEXT("IMAGE/37330", "Video Title"));
    ui->label_datePosition->setText(GET_TEXT("IMAGE/37331", "Date Position"));
    ui->label_titlePosition->setText(GET_TEXT("IMAGE/37332", "Title Position"));
    ui->label_dateFormat->setText(GET_TEXT("WIZARD/11015", "Date Format"));
    ui->checkBox_showTimestamp->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->checkBox_showTitle->setText(GET_TEXT("COMMON/1009", "Enable"));

    ui->comboBox_stream->setItemText(0, GET_TEXT("IMAGE/37343", "All Streams"));
    ui->comboBox_stream->setItemText(1, GET_TEXT("IMAGE/37333", "Primary Stream"));
    ui->comboBox_stream->setItemText(2, GET_TEXT("CHANNELMANAGE/30057", "Secondary Stream"));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));

    ui->labelFontSize->setText(GET_TEXT("POS/130013", "Font Size"));
}

void ImagePageOsd::on_comboBox_stream_activated(int index)
{
    const OSD_STREAM_TYPE &stream_type = static_cast<OSD_STREAM_TYPE>(ui->comboBox_stream->itemData(index).toInt());
    m_ovf_osd.stream_type = stream_type;

    switch (stream_type) {
    case OSD_ALL_STREAM:
    case OSD_MAIN_STREAM:
        ui->checkBox_showTitle->setChecked(m_ovf_osd.enable_text);
        ui->lineEdit_videoTitle->setText(m_ovf_osd.text_name);
        ui->comboBox_titlePosition->setCurrentIndexFromData(m_ovf_osd.textpos);
        ui->checkBox_showTimestamp->setChecked(m_ovf_osd.enable_date);
        ui->comboBox_datePosition->setCurrentIndexFromData(m_ovf_osd.datepos);
        ui->comboBox_dateFormat->setCurrentIndexFromData(m_ovf_osd.dateformat);
        ui->comboBoxFontSize->setCurrentIndexFromData(m_ovf_osd.fontSize);
        break;
    case OSD_SUB_STREAM:
        ui->checkBox_showTitle->setChecked(m_ovf_osd.enable_text_sub);
        ui->lineEdit_videoTitle->setText(m_ovf_osd.text_name_sub);
        ui->comboBox_titlePosition->setCurrentIndexFromData(m_ovf_osd.textpos_sub);
        ui->checkBox_showTimestamp->setChecked(m_ovf_osd.enable_date_sub);
        ui->comboBox_datePosition->setCurrentIndexFromData(m_ovf_osd.datepos_sub);
        ui->comboBox_dateFormat->setCurrentIndexFromData(m_ovf_osd.dateformat_sub);
        ui->comboBoxFontSize->setCurrentIndexFromData(m_ovf_osd.fontSizeSub);
        break;
    case OSD_THD_STREAM:
        ui->checkBox_showTitle->setChecked(m_ovf_osd.enable_text_thd);
        ui->lineEdit_videoTitle->setText(m_ovf_osd.text_name_thd);
        ui->comboBox_titlePosition->setCurrentIndexFromData(m_ovf_osd.textpos_thd);
        ui->checkBox_showTimestamp->setChecked(m_ovf_osd.enable_date_thd);
        ui->comboBox_datePosition->setCurrentIndexFromData(m_ovf_osd.datepos_thd);
        ui->comboBox_dateFormat->setCurrentIndexFromData(m_ovf_osd.dateformat_thd);
        ui->comboBoxFontSize->setCurrentIndexFromData(m_ovf_osd.fontSizeThd);
        break;
    default:
        break;
    }
    statusChange();
    ui->lineEdit_videoTitle->setValid(true);
}

void ImagePageOsd::on_checkBox_showTitle_clicked(bool checked)
{
    const OSD_STREAM_TYPE &stream_type = static_cast<OSD_STREAM_TYPE>(ui->comboBox_stream->currentData().toInt());
    switch (stream_type) {
    case OSD_ALL_STREAM:
        m_ovf_osd.enable_text = checked;
        m_ovf_osd.enable_text_sub = checked;
        m_ovf_osd.enable_text_thd = checked;
        break;
    case OSD_MAIN_STREAM:
        m_ovf_osd.enable_text = checked;
        break;
    case OSD_SUB_STREAM:
        m_ovf_osd.enable_text_sub = checked;
        break;
    case OSD_THD_STREAM:
        m_ovf_osd.enable_text_thd = checked;
        break;
    default:
        break;
    }
    statusChange();
}

void ImagePageOsd::on_lineEdit_videoTitle_textEdited(const QString &text)
{
    const OSD_STREAM_TYPE &stream_type = static_cast<OSD_STREAM_TYPE>(ui->comboBox_stream->currentData().toInt());
    switch (stream_type) {
    case OSD_ALL_STREAM:
        snprintf(m_ovf_osd.text_name, sizeof(m_ovf_osd.text_name), "%s", text.toStdString().c_str());
        snprintf(m_ovf_osd.text_name_sub, sizeof(m_ovf_osd.text_name_sub), "%s", text.toStdString().c_str());
        snprintf(m_ovf_osd.text_name_thd, sizeof(m_ovf_osd.text_name_thd), "%s", text.toStdString().c_str());
        break;
    case OSD_MAIN_STREAM:
        snprintf(m_ovf_osd.text_name, sizeof(m_ovf_osd.text_name), "%s", text.toStdString().c_str());
        break;
    case OSD_SUB_STREAM:
        snprintf(m_ovf_osd.text_name_sub, sizeof(m_ovf_osd.text_name_sub), "%s", text.toStdString().c_str());
        break;
    case OSD_THD_STREAM:
        snprintf(m_ovf_osd.text_name_thd, sizeof(m_ovf_osd.text_name_thd), "%s", text.toStdString().c_str());
        break;
    default:
        break;
    }
}

void ImagePageOsd::on_comboBox_titlePosition_activated(int index)
{
    const int position = ui->comboBox_titlePosition->itemData(index).toInt();

    const OSD_STREAM_TYPE &stream_type = static_cast<OSD_STREAM_TYPE>(ui->comboBox_stream->currentData().toInt());
    switch (stream_type) {
    case OSD_ALL_STREAM:
        m_ovf_osd.textpos = position;
        m_ovf_osd.textpos_sub = position;
        m_ovf_osd.textpos_thd = position;
        break;
    case OSD_MAIN_STREAM:
        m_ovf_osd.textpos = position;
        break;
    case OSD_SUB_STREAM:
        m_ovf_osd.textpos_sub = position;
        break;
    case OSD_THD_STREAM:
        m_ovf_osd.textpos_thd = position;
        break;
    default:
        break;
    }
}

void ImagePageOsd::on_checkBox_showTimestamp_clicked(bool checked)
{
    const OSD_STREAM_TYPE &stream_type = static_cast<OSD_STREAM_TYPE>(ui->comboBox_stream->currentData().toInt());
    m_ovf_osd.stream_type = stream_type;
    switch (stream_type) {
    case OSD_ALL_STREAM:
        m_ovf_osd.enable_date = checked;
        m_ovf_osd.enable_date_sub = checked;
        m_ovf_osd.enable_date_thd = checked;
        break;
    case OSD_MAIN_STREAM:
        m_ovf_osd.enable_date = checked;
        break;
    case OSD_SUB_STREAM:
        m_ovf_osd.enable_date_sub = checked;
        break;
    case OSD_THD_STREAM:
        m_ovf_osd.enable_date_thd = checked;
        break;
    default:
        break;
    }
    statusChange();
}

void ImagePageOsd::on_comboBox_datePosition_activated(int index)
{
    const int position = ui->comboBox_datePosition->itemData(index).toInt();

    const OSD_STREAM_TYPE &stream_type = static_cast<OSD_STREAM_TYPE>(ui->comboBox_stream->currentData().toInt());
    m_ovf_osd.stream_type = stream_type;
    switch (stream_type) {
    case OSD_ALL_STREAM:
        m_ovf_osd.datepos = position;
        m_ovf_osd.datepos_sub = position;
        m_ovf_osd.datepos_thd = position;
        break;
    case OSD_MAIN_STREAM:
        m_ovf_osd.datepos = position;
        break;
    case OSD_SUB_STREAM:
        m_ovf_osd.datepos_sub = position;
        break;
    case OSD_THD_STREAM:
        m_ovf_osd.datepos_thd = position;
        break;
    default:
        break;
    }
}

void ImagePageOsd::on_comboBox_dateFormat_activated(int index)
{
    const int dateFormat = ui->comboBox_dateFormat->itemData(index).toInt();

    const OSD_STREAM_TYPE &stream_type = static_cast<OSD_STREAM_TYPE>(ui->comboBox_stream->currentData().toInt());
    m_ovf_osd.stream_type = stream_type;
    switch (stream_type) {
    case OSD_ALL_STREAM:
        m_ovf_osd.dateformat = dateFormat;
        m_ovf_osd.dateformat_sub = dateFormat;
        m_ovf_osd.dateformat_thd = dateFormat;
        break;
    case OSD_MAIN_STREAM:
        m_ovf_osd.dateformat = dateFormat;
        break;
    case OSD_SUB_STREAM:
        m_ovf_osd.dateformat_sub = dateFormat;
        break;
    case OSD_THD_STREAM:
        m_ovf_osd.dateformat_thd = dateFormat;
        break;
    default:
        break;
    }
}

bool ImagePageOsd::isInputValid()
{
    if (!ui->lineEdit_videoTitle->isEnabled()) {
        return true;
    }
    if (!ui->lineEdit_videoTitle->checkValid()) {
        return false;
    }
    return true;
}

void ImagePageOsd::statusChange()
{
    ui->lineEdit_videoTitle->setEnabled(ui->checkBox_showTitle->isChecked());
    ui->comboBox_titlePosition->setEnabled(ui->checkBox_showTitle->isChecked());

    ui->comboBox_dateFormat->setEnabled(ui->checkBox_showTimestamp->isChecked());
    ui->comboBox_datePosition->setEnabled(ui->checkBox_showTimestamp->isChecked());
}

void ImagePageOsd::on_pushButton_copy_clicked()
{
    m_copyChannelList.clear();

    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == QDialog::Accepted) {
        m_copyChannelList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(on_pushButton_apply_clicked()));
    }
}

void ImagePageOsd::on_pushButton_apply_clicked()
{
    if (!isInputValid()) {
        return;
    }
    const OSD_STREAM_TYPE &stream_type = static_cast<OSD_STREAM_TYPE>(ui->comboBox_stream->currentData().toInt());
    if (stream_type == OSD_ALL_STREAM) {
        m_ovf_osd.enable_text_sub = m_ovf_osd.enable_text;
        m_ovf_osd.enable_text_thd = m_ovf_osd.enable_text;
        snprintf(m_ovf_osd.text_name_sub, sizeof(m_ovf_osd.text_name_sub), "%s", m_ovf_osd.text_name);
        snprintf(m_ovf_osd.text_name_thd, sizeof(m_ovf_osd.text_name_thd), "%s", m_ovf_osd.text_name);
        m_ovf_osd.textpos_sub = m_ovf_osd.textpos;
        m_ovf_osd.textpos_thd = m_ovf_osd.textpos;
        m_ovf_osd.enable_date_sub = m_ovf_osd.enable_date;
        m_ovf_osd.enable_date_thd = m_ovf_osd.enable_date;
        m_ovf_osd.datepos_sub = m_ovf_osd.datepos;
        m_ovf_osd.datepos_thd = m_ovf_osd.datepos;
        m_ovf_osd.dateformat_sub = m_ovf_osd.dateformat;
        m_ovf_osd.dateformat_thd = m_ovf_osd.dateformat;
        m_ovf_osd.fontSizeSub = m_ovf_osd.fontSize;
        m_ovf_osd.fontSizeThd = m_ovf_osd.fontSize;
    }

    if (m_copyChannelList.isEmpty()) {
        m_copyChannelList.append(m_currentChannel);
    }
    sendCopyData();

    //
    //MsWaitting::showGlobalWait();
}

void ImagePageOsd::on_pushButton_back_clicked()
{
    back();
}

void ImagePageOsd::on_comboBoxFontSize_activated(int index)
{
    const int fontSize = ui->comboBoxFontSize->itemData(index).toInt();

    const OSD_STREAM_TYPE &stream_type = static_cast<OSD_STREAM_TYPE>(ui->comboBox_stream->currentData().toInt());
    m_ovf_osd.stream_type = stream_type;
    switch (stream_type) {
    case OSD_ALL_STREAM:
        m_ovf_osd.fontSize = static_cast<FontSize>(fontSize);
        m_ovf_osd.fontSizeSub = static_cast<FontSize>(fontSize);
        m_ovf_osd.fontSizeThd = static_cast<FontSize>(fontSize);
        break;
    case OSD_MAIN_STREAM:
        m_ovf_osd.fontSize = static_cast<FontSize>(fontSize);
        break;
    case OSD_SUB_STREAM:
        m_ovf_osd.fontSizeSub = static_cast<FontSize>(fontSize);
        break;
    case OSD_THD_STREAM:
        m_ovf_osd.fontSizeThd = static_cast<FontSize>(fontSize);
        break;
    }
}
