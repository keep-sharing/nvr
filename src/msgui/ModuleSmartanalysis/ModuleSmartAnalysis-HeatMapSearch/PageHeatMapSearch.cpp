#include "PageHeatMapSearch.h"
#include "qglobal.h"
#include "ui_PageHeatMapSearch.h"
#include "DrawView.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyFileSystemDialog.h"
#include "centralmessage.h"
#include "ptzdatamanager.h"
#include <QtDebug>
extern "C" {
#include "msg.h"

}

PageHeatMapSearch::PageHeatMapSearch(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::HeatMapAnalysis)
{
    ui->setupUi(this);

    ui->checkBoxGroupChannel->setCountFromChannelName(qMsNvr->maxChannel());

    ui->comboBox_mainType->clear();
    ui->comboBox_mainType->addItem(GET_TEXT("HEATMAP/104008", "Space Heat Map"), 0);
    ui->comboBox_mainType->addItem(GET_TEXT("HEATMAP/104009", "Time Heat Map"), 1);

    ui->comboBox_reportType->addItem(GET_TEXT("HEATMAP/104011", "Daily Report"), 0);
    ui->comboBox_reportType->addItem(GET_TEXT("HEATMAP/104012", "Weekly Report"), 1);
    ui->comboBox_reportType->addItem(GET_TEXT("HEATMAP/104013", "Monthly Report"), 2);
    ui->comboBox_reportType->addItem(GET_TEXT("HEATMAP/104014", "Annual Report"), 3);

    m_drawView = new DrawView(this);
    m_drawScene = new DrawSceneHeatMap(this);
    m_drawView->setScene(m_drawScene);
    ui->gridLayoutResult->addWidget(m_drawView);

    m_heatMapThread = new HeatMapThread();
    connect(m_heatMapThread, SIGNAL(imageFinished(int, int, QImage, QImage)), this, SLOT(onImageFinished(int, int, QImage, QImage)));
    connect(m_heatMapThread, SIGNAL(saveFinished(bool)), this, SLOT(onSaveSpaceHeatMapFinished(bool)));

    onLanguageChanged();
}

PageHeatMapSearch::~PageHeatMapSearch()
{
    m_heatMapThread->stop();
    delete m_heatMapThread;
    delete ui;
}

void PageHeatMapSearch::initializeData()
{
    ui->comboBox_mainType->setCurrentIndex(0);
    ui->comboBox_reportType->setCurrentIndex(0);

    ui->dateEditStartDate->setDate(QDate::currentDate());
    ui->timeEditStartTime->setTime(QTime(0, 0));
    ui->stackedWidget->setCurrentWidget(ui->pageSearch);

    ui->checkBoxGroupChannel->clearCheck();
}

void PageHeatMapSearch::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
        break;
    case RESPONSE_FLAG_GET_IPC_SNAPHOST:
        ON_RESPONSE_FLAG_GET_IPC_SNAPHOST(message);
        break;
    case RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT:
        ON_RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT(message);
        break;
    case RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT:
        ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(message);
        break;
    }
}

void PageHeatMapSearch::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    resp_image_display *image_display = static_cast<resp_image_display *>(message->data);
    if (!image_display) {
        qWarning() << QString("HeatMapAnalysis::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY, data is null.");
    } else {
        m_drawHeatMapDataMap[m_indexChannel].corridorMode = image_display->image.corridormode;
        m_drawHeatMapDataMap[m_indexChannel].imageRotation = image_display->image.imagerotation;
        m_drawHeatMapDataMap[m_indexChannel].lencorrect = image_display->image.lencorrect;
    }

    m_eventLoop.exit();
}

void PageHeatMapSearch::ON_RESPONSE_FLAG_GET_IPC_SNAPHOST(MessageReceive *message)
{
    //qDebug() << "HeatMapAnalysis::ON_RESPONSE_FLAG_GET_IPC_SNAPHOST, data:" << message->data << ", size:" << message->header.size;
    RespSnapshotChatHeader *header = static_cast<RespSnapshotChatHeader *>(message->data);
    if (header) {
        if (header->result == -1) {
            m_decoding = 1;
            m_selectList.removeAll(header->chnid);
            m_drawHeatMapDataMap.remove(header->chnid);
        } else if (header->result == -2) {
            m_selectList.removeAll(header->chnid);
            m_drawHeatMapDataMap.remove(header->chnid);
        } else {
            message->image1 = QImage::fromData(static_cast<uchar *>(message->data) + sizeof(RespSnapshotChatHeader), message->header.size - sizeof(RespSnapshotChatHeader));
            m_drawHeatMapDataMap[header->chnid].backgroundImage = message->image1;
        }
    }
    m_count--;
    if (m_count == 0) {
        m_eventLoop.exit();
    }
}

void PageHeatMapSearch::ON_RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT(MessageReceive *message)
{
    char *report = static_cast<char *>(message->data);
    if (report) {

        //qDebug() << QString("HeatMapAnalysis::ON_RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT, size: %1").arg(message->header.size);
        QByteArray ba(report, message->header.size);
        QString strText(ba);
        m_drawHeatMapDataMap[m_indexChannel].text = strText;
        m_eventLoop.exit();
    } else {
        qWarning() << QString("HeatMapAnalysis::ON_RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT, data is null.");
        m_eventLoop.exit(-1);
    }
}

void PageHeatMapSearch::ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(MessageReceive *message)
{
    int result = *((int *)message->data);
    m_eventLoop.exit(result);
}

void PageHeatMapSearch::clearInfo()
{
    m_drawScene->clearInfo();
}

bool PageHeatMapSearch::isSupport(int channel)
{
    if (!isConnected(channel)) {
        return false;
    }
    bool isSupport = false;
    sendMessage(REQUEST_FLAG_GET_IPC_HEATMAP_SUPPORT, &channel, sizeof(int));
    // int result = m_eventLoop.exec();
    // if (result != HEATMAP_SUPPORT_NO) {
    //     isSupport = true;
    // }
    // if (result != HEATMAP_SUPPORT_YES && ui->comboBox_mainType->currentIntData() == 0) {
    //     isSupport = false;
    // }
    // if (!isSupport) {
    //     return false;
    // }

    sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &channel, sizeof(int));
    //m_eventLoop.exec();

    if (qMsNvr->isFisheye(channel)) {
        m_drawHeatMapDataMap[channel].mode = ModeFisheye;
    } else {
        m_drawHeatMapDataMap[channel].mode = ModePanoramicMiniBullet;
    }

    return isSupport;
}

bool PageHeatMapSearch::isConnected(int channel)
{
    //
    if (LiveView::instance()->isChannelConnected(channel)) {
        return true;
    } else {
        return false;
    }
}

void PageHeatMapSearch::onLanguageChanged()
{
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelChannel2->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->label_mainType->setText(GET_TEXT("HEATMAP/104007", "Main Type"));
    ui->label_reportType->setText(GET_TEXT("HEATMAP/104010", "Report Type"));
    ui->label_startTime->setText(GET_TEXT("HEATMAP/104015", "Start Time"));

    ui->pushButton_search->setText(GET_TEXT("HEATMAP/104017", "Search"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->pushButtonBackupAll->setText(GET_TEXT("COMMONBACKUP/100002", "Backup All"));
    ui->pushButtonBackup->setText(GET_TEXT("PROFILE/76001", "Backup"));
    ui->pushButtonResultBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PageHeatMapSearch::onImageFinished(int max, int min, QImage colorImage, QImage heatmapImage)
{
    struct ImageParameters imageParamerters;
    imageParamerters.max = max;
    imageParamerters.min = min;
    imageParamerters.colorImage = colorImage;
    imageParamerters.heatmapImage = heatmapImage;
    //m_drawHeatMapDataMap[m_currentChannel].backgroundImage = heatmapImage;
    //m_imageParametersMap.insert(m_currentChannel, imageParamerters);
    if (!m_isBackuping) {
        m_drawScene->showSpaceHeatMap(imageParamerters);
        //closeWait();
    }
}

void PageHeatMapSearch::onSaveSpaceHeatMapFinished(bool result)
{
    Q_UNUSED(result)
    //m_eventLoop.exit(result);
}

void PageHeatMapSearch::on_comboBoxChannel_activated(int index)
{
    Q_UNUSED(index);
    clearInfo();
    m_currentChannel = ui->comboBoxChannel->currentIntData();
    int mainType = ui->comboBox_mainType->currentIntData();
    switch (mainType) {
    case 0: {
        m_heatMapThread->setBackgroundImage(m_drawHeatMapDataMap.value(m_currentChannel).backgroundImage);
        showWait(this);
        m_heatMapThread->makeHeatMap(m_drawHeatMapDataMap.value(m_currentChannel));
        break;
    }
    case 1: {
        int reportType = ui->comboBox_reportType->currentData().toInt();
        QDateTime dateTime = QDateTime(ui->dateEditStartDate->date(), ui->timeEditStartTime->time());
        qDebug() << "text:" << m_drawHeatMapDataMap[m_currentChannel].text << "\n"
                 << "type:" << reportType << "\n"
                 << "time" << dateTime;
        m_drawScene->showTimeHeatMap(m_drawHeatMapDataMap[m_currentChannel].text, reportType, dateTime);
        break;
    }
    }
}

void PageHeatMapSearch::on_pushButton_search_clicked()
{
    m_drawHeatMapDataMap.clear();
    ui->comboBoxChannel->clear();
    m_selectList = ui->checkBoxGroupChannel->checkedList();
    m_count = m_selectList.count();
    if (m_selectList.isEmpty()) {
        ShowMessageBox(GET_TEXT("FISHEYE/12018", "Please select at least one channel."));
        return;
    }
    int maxSize = m_selectList.count();
    showWait(this);
    int mainType = ui->comboBox_mainType->currentData().toInt();
    m_decoding = 0;
    for (int i = 0; i < m_selectList.count(); i++) {
        int channel = m_selectList.at(i);
        struct DrawHeatMapData heatMapData;
        m_drawHeatMapDataMap.insert(channel, heatMapData);
        if (!isSupport(channel)) {
            m_selectList.removeAll(channel);
            i--;
            m_count--;
            m_drawHeatMapDataMap.remove(channel);
            continue;
        }
        //
        if (mainType == 0) {
            //qDebug() << QString("REQUEST_FLAG_GET_IPC_SNAPHOST, chanid: %1").arg(channel);
            sendMessage(REQUEST_FLAG_GET_IPC_SNAPHOST, &channel, sizeof(int));
        }
    }
    //等待截图
    if (mainType == 0 && m_count > 0) {
        //m_eventLoop.exec();
    }

    memset(&m_report, 0, sizeof(ms_heat_map_report));
    m_report.chnid = m_indexChannel;
    m_report.mainType = mainType;
    m_report.subType = ui->comboBox_reportType->currentData().toInt();
    snprintf(m_report.pTime, sizeof(m_report.pTime), "%s", QDateTime(ui->dateEditStartDate->date(), ui->timeEditStartTime->time()).toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
    for (auto channel : m_selectList) {
        m_indexChannel = channel;
        m_report.chnid = m_indexChannel;
        //qDebug() << QString("REQUEST_FLAG_SEARCH_HEAT_MAP_REPORT, chanid: %1, mainType: %2, subType: %3, pTime: %4").arg(m_report.chnid).arg(m_report.mainType).arg(m_report.subType).arg(m_report.pTime);
        sendMessage(REQUEST_FLAG_SEARCH_HEAT_MAP_REPORT, &m_report, sizeof(ms_heat_map_report));
        // int result = m_eventLoop.exec();
        // if (result < 0) {
        //     //qWarning() << "REQUEST_FLAG_SEARCH_HEAT_MAP_REPORT failed, channel:" << channel;
        //     m_selectList.removeAll(m_indexChannel);
        //     m_drawHeatMapDataMap.remove(m_indexChannel);
        //     continue;
        // }
    }
    //closeWait();
    if (m_decoding && m_selectList.count() == 0) {
        ShowMessageBox(GET_TEXT("LIVEVIEW/178000", "If this functionality is required, the resolution must be within the range of 4000x4000"));
        return;
    } else if (m_decoding && m_selectList.count() < maxSize) {
        MessageBox::information(this, GET_TEXT("LIVEVIEW/178000", "If this functionality is required, the resolution must be within the range of 4000x4000"));
    } else if (m_selectList.count() == 0) {
        ShowMessageBox(GET_TEXT("HEATMAP/104018", "No matching data."));
        return;
    } else if (m_selectList.count() < maxSize) {
        MessageBox::information(this, GET_TEXT("HEATMAP/147002", "No matching data of some channels."));
    }
    for (auto channel : m_selectList) {
        ui->comboBoxChannel->addItem(QString("%1").arg(channel + 1), channel);
    }
    ui->stackedWidget->setCurrentWidget(ui->pageResult);
    on_comboBoxChannel_activated(0);
    ui->pushButton_search->clearUnderMouse();
}

void PageHeatMapSearch::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PageHeatMapSearch::on_pushButtonBackupAll_clicked()
{
    QString dirPath = MyFileSystemDialog::instance()->getOpenDirectory();
    if (dirPath.isEmpty()) {
        return;
    }
    showWait(this);
    int result = 0;
    int type = ui->comboBox_mainType->currentData().toInt();
    m_isBackuping = true;
    switch (type) {
    case 0: {
        for (auto channel : m_selectList) {
            m_currentChannel = channel;
            QString filePath = QString("%1/NVR-CH%2_space_%3.png").arg(dirPath).arg(m_currentChannel + 1, 2, 10, QLatin1Char('0')).arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
            m_heatMapThread->setBackgroundImage(m_drawHeatMapDataMap.value(m_currentChannel).backgroundImage);
            m_heatMapThread->makeHeatMap(m_drawHeatMapDataMap.value(m_currentChannel));
            m_heatMapThread->saveSpaceHeatMap(filePath);
            // if (m_eventLoop.exec()) {
            //     result |= 1 << 0;
            // } else {
            //     result |= 1 << 1;
            // }
        }
        break;
    }
    case 1: {
        int reportType = ui->comboBox_reportType->currentData().toInt();
        QDateTime dateTime = QDateTime(ui->dateEditStartDate->date(), ui->timeEditStartTime->time());
        for (auto channel : m_selectList) {
            m_currentChannel = channel;
            QString filePath = QString("%1/NVR-CH%2_time_%3.csv").arg(dirPath).arg(m_currentChannel + 1, 2, 10, QLatin1Char('0')).arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
            if (m_drawScene->saveTimeHeatMap(m_drawHeatMapDataMap[m_currentChannel].text, reportType, dateTime, filePath)) {
                result |= 1 << 0;
            } else {
                result |= 1 << 1;
            }
        }
        break;
    }
    default:
        break;
    }

    m_currentChannel = ui->comboBoxChannel->currentIntData();
    m_isBackuping = false;
    //closeWait();
    if (result == 1) {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145000", "Backup successfully."));
    } else if (result == 3) {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145001", "Failed to backup some results."));
    } else {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145002", "Failed to backup."));
    }
}

void PageHeatMapSearch::on_pushButtonBackup_clicked()
{
    QString dirPath = MyFileSystemDialog::instance()->getOpenDirectory();
    if (dirPath.isEmpty()) {
        return;
    }
    int type = ui->comboBox_mainType->currentData().toInt();
    switch (type) {
    case 0: {
        showWait(this);

        QString filePath = QString("%1/NVR-CH%2_space_%3.png").arg(dirPath).arg(m_currentChannel + 1, 2, 10, QLatin1Char('0')).arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
        m_heatMapThread->saveSpaceHeatMap(filePath);

        //int result = m_eventLoop.exec();
        //closeWait();
        // if (result == 1) {
        //     ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145000", "Backup successfully."));
        // } else {
        //     ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145002", "Failed to backup."));
        // }
        break;
    }
    case 1: {
        QString filePath = QString("%1/NVR-CH%2_time_%3.csv").arg(dirPath).arg(m_currentChannel + 1, 2, 10, QLatin1Char('0')).arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
        bool result = m_drawScene->saveTimeHeatMap(filePath);
        if (result) {
            ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145000", "Backup successfully."));
        } else {
            ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145002", "Failed to backup."));
        }
        break;
    }
    default:
        break;
    }
}

void PageHeatMapSearch::on_pushButtonResultBack_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageSearch);
}
