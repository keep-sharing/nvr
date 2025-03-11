#include "imagepagedaynight.h"
#include "ui_imagepagedaynight.h"
#include "DayNightSchedule.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsSdkVersion.h"
#include "MsWaitting.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "daynightscheedit.h"
#include <QtDebug>

ImagePageDayNight::ImagePageDayNight(QWidget *parent)
    : AbstractImagePage(parent)
    , ui(new Ui::ImagePageDayNight)
{
    ui->setupUi(this);

    //initialize table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("IMAGE/162001", "Template");
    headerList << GET_TEXT("IMAGE/37300", "Time");
    headerList << GET_TEXT("IMAGE/37301", "Exposure Level");
    headerList << GET_TEXT("IMAGE/37302", "Minimum Shutter");
    headerList << GET_TEXT("IMAGE/37303", "Maximum Shutter");
    headerList << GET_TEXT("IMAGE/37304", "Limit Gain Level");
    headerList << GET_TEXT("IMAGE/37305", "IR-CUT Latency");
    headerList << GET_TEXT("IMAGE/37306", "IR-CUT");
    headerList << GET_TEXT("IMAGE/37307", "IR LED");
    headerList << GET_TEXT("IMAGE/37311", "Color Mode");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->hideColumn(ColumnCheck);

    ui->pushButtonTemplateSchedule->setText(GET_TEXT("IMAGE/162002", "Template Schedule"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));

    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    //sort
    ui->tableView->setSortingEnabled(false);
    //connect
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(slotEditDayNightInfo(int, int)));

    m_dayNightSche = new DayNightSchedule(this);
}

ImagePageDayNight::~ImagePageDayNight()
{
    delete ui;
}

void ImagePageDayNight::initializeData(int channel)
{
    m_channel = channel;

    clearSettings();
    setSettingsEnable(false);
    //MsWaitting::showGlobalWait();

    if (!isChannelConnected()) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        //MsWaitting::closeGlobalWait();
        return;
    }
    if (!qMsNvr->isMsCamera(m_channel)) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        //MsWaitting::closeGlobalWait();
        return;
    }

    setSettingsEnable(true);

    m_isWhite = false;
    sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_channel, sizeof(int));
}

void ImagePageDayNight::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_DAY_NIGHT_INFO:
        ON_RESPONSE_FLAG_GET_DAY_NIGHT_INFO(message);
        break;
    case RESPONSE_FLAG_GET_IPC_COMMON_PARAM:
        ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(message);
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
        break;
    default:
        break;
    }
}

void ImagePageDayNight::clearSettings()
{
    ui->widgetMessage->hideMessage();
    ui->tableView->clearContent();
}

void ImagePageDayNight::setSettingsEnable(bool enable)
{
    ui->tableView->setEnabled(enable);
    ui->pushButtonTemplateSchedule->setEnabled(enable);
    ui->pushButton_copy->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
}

void ImagePageDayNight::ON_RESPONSE_FLAG_GET_DAY_NIGHT_INFO(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    set_image_day_night_str *data = static_cast<set_image_day_night_str *>(message->data);
    if (!data) {
        qWarning() << "ImagePageDayNight::ON_RESPONSE_FLAG_GET_DAY_NIGHT_INFO, data is null.";
        return;
    }
    memset(&m_info, 0, sizeof(set_image_day_night_str));
    memcpy(&m_info, data, sizeof(set_image_day_night_str));
    initDayNight();
}

void ImagePageDayNight::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    resp_image_display *data = (resp_image_display *)message->data;
    if (!data) {
        qWarning() << "ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY, data is null.";
        //MsWaitting::closeGlobalWait();
        return;
    }
    m_chipninterface = atoi(data->image.chipninterface);
    if (m_chipninterface & m_PWM_1IR_1W || m_chipninterface & m_PWM_2IR_1W || m_chipninterface & m_PWM_2IR_1NW || m_chipninterface & m_PWM_1IR_2W) {
        m_isWhite = true;
    }
    m_fullcolorSupport = data->image.whiteled.support;

    sendMessage(REQUEST_FLAG_GET_DAY_NIGHT_INFO, &m_channel, sizeof(int));
    getCommonParam("sdkversion");
    getCommonParam("exposurectrl");
}

void ImagePageDayNight::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message)
{
    resp_http_common_param *param = (resp_http_common_param *)message->data;
    if (!param) {
        qWarning() << "ImagePageDayNight::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM, data is null.";
        return;
    }
    if (param->res == 0) {
        if (QString(param->info.key) == "sdkversion") {
            m_sdkversion = QString("%1").arg(param->info.value);
            ui->pushButtonTemplateSchedule->setEnabled(true);
            if (MsSdkVersion("2.4.02") > MsSdkVersion(m_sdkversion)) {
                ui->pushButtonTemplateSchedule->setEnabled(false);
            }
        }
        if (QString(param->info.key) == "exposurectrl") {
            m_exposureCtrl = atoi(param->info.value);
            changeShutter();
        }
    }
}

QString ImagePageDayNight::showTime(int id)
{
    QString time;
    QString startMinute;
    QString endMinute;
    switch (id) {
    case 0:
        time.append("Night");
        break;
    case 1:
        time.append("Day");
        break;
    default:
        time = "-";
        break;
    }
    return time;
}

QString ImagePageDayNight::showIRLedOffOnStatus(int type)
{
    QString show;
    switch (type) {
    case 0:
        if (m_isWhite)
            show.append(GET_TEXT("IMAGE/37366", "All LED Off"));
        else
            show.append(GET_TEXT("IMAGE/37316", "Off"));
        break;
    case 1:
        if (m_isWhite)
            show.append(GET_TEXT("IMAGE/37367", "IR LED On"));
        else
            show.append(GET_TEXT("IMAGE/37317", "On"));
        break;
    case 2:
        show.append(GET_TEXT("IMAGE/37368", "White LED On"));
        break;
    }
    return show;
}

QString ImagePageDayNight::showOffOnStatus(int type)
{
    QString show;
    switch (type) {
    case 0:
        show.append(GET_TEXT("IMAGE/37316", "Off"));
        break;
    case 1:
        show.append(GET_TEXT("IMAGE/37317", "On"));
        break;
    }
    return show;
}

QString ImagePageDayNight::showColorMode(int type)
{
    QString show;
    switch (type) {
    case 0:
        show.append("B/W");
        break;
    case 1:
        show.append("Color");
        break;
    }
    return show;
}

QString ImagePageDayNight::showLatency(int id)
{
    QString show;
    switch (id) {
    case 0:
        show = QString("%1s").arg(m_info.imgSingle.image[id].irCutLatency);
        break;
    case 1:
        show = QString("%1s").arg(m_info.imgSingle.image[id].irCutLatency);
        break;
    default:
        show = QString("%1s").arg(m_info.imgSingle.image[id].irCutLatency + 1);
        break;
    }
    return show;
}

QString ImagePageDayNight::showShutter(int num)
{
    QString show;
    if (!m_exposureCtrl) {
        switch (num) {
        case 0:
            show.append("1/5");
            break;
        case 1:
            show.append("1/15");
            break;
        case 2:
            show.append("1/30");
            break;
        case 3:
            show.append("1/60");
            break;
        case 4:
            show.append("1/120");
            break;
        case 5:
            show.append("1/250");
            break;
        case 6:
            show.append("1/500");
            break;
        case 7:
            show.append("1/750");
            break;
        case 8:
            show.append("1/1000");
            break;
        case 9:
            show.append("1/2000");
            break;
        case 10:
            show.append("1/4000");
            break;
        case 11:
            show.append("1/10000");
            break;
        case 12:
            show.append("1/100000");
            break;
        case 13:
            show.append("1");
            break;
        default:
            break;
        }
    } else {
        switch (num) {
        case 0:
            show.append("1/5");
            break;
        case 1:
            show.append("1/10");
            break;
        case 2:
            show.append("1/25");
            break;
        case 3:
            show.append("1/50");
            break;
        case 4:
            show.append("1/100");
            break;
        case 5:
            show.append("1/250");
            break;
        case 6:
            show.append("1/500");
            break;
        case 7:
            show.append("1/750");
            break;
        case 8:
            show.append("1/1000");
            break;
        case 9:
            show.append("1/2000");
            break;
        case 10:
            show.append("1/4000");
            break;
        case 11:
            show.append("1/10000");
            break;
        case 12:
            show.append("1/100000");
            break;
        case 13:
            show.append("1");
            break;
        default:
            break;
        }
    }
    return show;
}

void ImagePageDayNight::deleteListAll()
{
    int i = 0;
    int row = ui->tableView->rowCount();
    for (i = row; i >= 0; i--)
        ui->tableView->removeRow(i);
}

void ImagePageDayNight::changeShutter()
{
    int i = 0, row = 0;
    for (i = 0; i < MAX_IMAGE_DAY_NIGHT; i++) {
        if (!m_info.imgSingle.image[i].enable)
            continue;

        QString minShutter = showShutter(m_info.imgSingle.image[i].minShutter);
        ui->tableView->setItemText(row, ColumnMinimumShutter, minShutter);

        QString maxShutter = showShutter(m_info.imgSingle.image[i].maxShutter);
        ui->tableView->setItemText(row, ColumnMaximumShutter, maxShutter);

        row++;
    }
}

void ImagePageDayNight::initDayNight()
{
    int i = 0, row = 0;
    deleteListAll();
    for (i = 0; i < MAX_IMAGE_DAY_NIGHT; i++) {

        ui->tableView->insertRow(row);
        ui->tableView->setItemIntValue(row, ColumnId, i);

        QString templateStr = i < 2 ? "-" : QString("%1").arg(i - 1);
        ui->tableView->setItemText(row, ColumnId, templateStr);

        QString time = showTime(i);
        ui->tableView->setItemText(row, ColumnTime, time);

        ui->tableView->setItemIntValue(row, ColumnExposureLevel, m_info.imgSingle.image[i].exposureLevel);

        QString minShutter = showShutter(m_info.imgSingle.image[i].minShutter);
        ui->tableView->setItemText(row, ColumnMinimumShutter, minShutter);

        QString maxShutter = showShutter(m_info.imgSingle.image[i].maxShutter);
        ui->tableView->setItemText(row, ColumnMaximumShutter, maxShutter);

        ui->tableView->setItemIntValue(row, ColumnLimitGainLevel, m_info.imgSingle.image[i].gainLevel);

        if (m_fullcolorSupport) {
            ui->tableView->setItemText(row, ColumnIrCutLatency, "-");
            ui->tableView->setItemText(row, ColumnIrCut, "-");
            ui->tableView->setItemText(row, ColumnIrLed, "-");
        } else {
            QString irCutLatency = showLatency(i);
            ui->tableView->setItemText(row, ColumnIrCutLatency, irCutLatency);

            QString irCutState = showOffOnStatus(m_info.imgSingle.image[i].irCutState);
            ui->tableView->setItemText(row, ColumnIrCut, irCutState);

            QString irLedState = showIRLedOffOnStatus(m_info.imgSingle.image[i].irLedState);
            ui->tableView->setItemText(row, ColumnIrLed, irLedState);
        }

        QString colorMode = showColorMode(m_info.imgSingle.image[i].colorMode);
        ui->tableView->setItemText(row, ColumnColorMode, colorMode);

        row++;
    }
}

void ImagePageDayNight::resizeEvent(QResizeEvent *)
{
    int columnWidth = width() / (ui->tableView->columnCount() - 1);
    for (int i = ColumnId; i < ColumnEdit; ++i) {
        ui->tableView->setColumnWidth(i, columnWidth);
    }
}

void ImagePageDayNight::closeWait()
{
    //MsWaitting::closeGlobalWait();
}

void ImagePageDayNight::slotEditDayNightInfo(int row, int column)
{
    int id = ui->tableView->itemIntValue(row, ColumnId);
    qDebug() << "row=" << row << ", column=" << column << ", id=" << id;

    if (column == ColumnEdit) {
        DayNightScheEdit dayNightSche(m_isWhite, this);
        dayNightSche.setExposureCtrl(m_exposureCtrl);
        connect(&dayNightSche, SIGNAL(sigEditSche()), this, SLOT(updateSche()));
        dayNightSche.initDayNightEditInfo(&m_info.imgSingle.image[id], id,  m_fullcolorSupport);
        dayNightSche.exec();
    }
}

void ImagePageDayNight::updateSche()
{
    //    initializeData(m_channel);
    initDayNight();
}

void ImagePageDayNight::saveDayNightSche()
{
    struct set_image_day_night_str info;
    memcpy(&info, &m_info, sizeof(set_image_day_night_str));
    info.chanid = m_copyChannelList.takeFirst();
    sendMessage(REQUEST_FLAG_SET_DAY_NIGHT_INFO, &info, sizeof(struct set_image_day_night_str));
}

void ImagePageDayNight::on_pushButton_back_clicked()
{
    back();
}

void ImagePageDayNight::on_pushButton_apply_clicked()
{
    //MsWaitting::showGlobalWait();

    if (m_copyChannelList.isEmpty()) {
        m_copyChannelList.append(m_channel);
    }

    do {
        saveDayNightSche();
        qApp->processEvents();
    } while (!m_copyChannelList.isEmpty());

    QTimer::singleShot(1000, this, SLOT(closeWait()));
}

void ImagePageDayNight::on_pushButton_copy_clicked()
{
    m_copyChannelList.clear();

    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_channel);
    int result = copy.exec();
    if (result == QDialog::Accepted) {
        m_copyChannelList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(on_pushButton_apply_clicked()));
    }
}

void ImagePageDayNight::getCommonParam(const QString &param)
{
    struct req_http_common_param common_param;
    memset(&common_param, 0, sizeof(struct req_http_common_param));

    common_param.chnid = m_channel;
    snprintf(common_param.info.key, sizeof(common_param.info.key), "%s", param.toStdString().c_str());

    sendMessage(REQUEST_FLAG_GET_IPC_COMMON_PARAM, &common_param, sizeof(struct req_http_common_param));
}

void ImagePageDayNight::on_pushButtonTemplateSchedule_clicked()
{
    m_dayNightSche->showAction(m_channel, &m_info);
    m_dayNightSche->exec();

    ui->pushButtonTemplateSchedule->clearUnderMouse();
}
