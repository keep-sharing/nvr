#include "PagePeopleCountingSearch.h"
#include "ui_PagePeopleCountingSearch.h"
#include "EventLoop.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "PeopleCountingCameraResult.h"
#include "PeopleCountingGroupResult.h"
#include "PeopleCountingRegionResult.h"
#include "centralmessage.h"

#define MAX_CHECKED 10

PagePeopleCountingSearch::PagePeopleCountingSearch(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PeopleCountingSearch)
{
    ui->setupUi(this);

    ui->comboBoxSearchType->beginEdit();
    ui->comboBoxSearchType->clear();
    ui->comboBoxSearchType->addItem(GET_TEXT("PEOPLECOUNTING_SEARCH/140000", "People Counting by Camera"), PEOPLECNT_SEARCH_BY_CAMERA);
    ui->comboBoxSearchType->addItem(GET_TEXT("PEOPLECOUNTING_SEARCH/140001", "People Counting by Group"), PEOPLECNT_SEARCH_BY_GROUP);
    ui->comboBoxSearchType->addItem(GET_TEXT("PEOPLECOUNTING_SEARCH/140002", "Regional People Counting"), PEOPLECNT_SEARCH_BY_REGION);
    ui->comboBoxSearchType->endEdit();

    //By Camera
    ui->checkBoxGroupChannelCamera->setCountFromChannelName(qMsNvr->maxChannel());

    ui->comboBoxReportTypeCamera->clear();
    ui->comboBoxReportTypeCamera->addItem(GET_TEXT("OCCUPANCY/74205", "Daily Report"), DailyReport);
    ui->comboBoxReportTypeCamera->addItem(GET_TEXT("OCCUPANCY/74206", "Weekly Report"), WeeklyReport);
    ui->comboBoxReportTypeCamera->addItem(GET_TEXT("OCCUPANCY/74207", "Monthly Report"), MonthlyReport);
    ui->comboBoxReportTypeCamera->addItem(GET_TEXT("HEATMAP/104014", "Annual Report"), YearlyReport);

    ui->comboBoxStatisticsTypeCamera->clear();
    ui->comboBoxStatisticsTypeCamera->addItem(GET_TEXT("OCCUPANCY/74209", "People Entered"), PeopleEntered);
    ui->comboBoxStatisticsTypeCamera->addItem(GET_TEXT("OCCUPANCY/74210", "People Exited"), PeopleExited);
    ui->comboBoxStatisticsTypeCamera->addItem(GET_TEXT("OCCUPANCY/74211", "Sum"), PeopleAll);

    ui->widgetLineType->setCount(5);
    QStringList typeList;
    typeList << GET_TEXT("PEOPLECOUNTING_SEARCH/145022", "Total")
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 1"
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 2"
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 3"
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 4";
    ui->widgetLineType->setCheckBoxTest(typeList);

    //By Group

    ui->comboBoxReportTypeGroup->clear();
    ui->comboBoxReportTypeGroup->addItem(GET_TEXT("OCCUPANCY/74205", "Daily Report"), DailyReport);
    ui->comboBoxReportTypeGroup->addItem(GET_TEXT("OCCUPANCY/74206", "Weekly Report"), WeeklyReport);
    ui->comboBoxReportTypeGroup->addItem(GET_TEXT("OCCUPANCY/74207", "Monthly Report"), MonthlyReport);

    ui->comboBoxStatisticsTypeGroup->clear();
    ui->comboBoxStatisticsTypeGroup->addItem(GET_TEXT("OCCUPANCY/74209", "People Entered"), PeopleEntered);
    ui->comboBoxStatisticsTypeGroup->addItem(GET_TEXT("OCCUPANCY/74210", "People Exited"), PeopleExited);
    ui->comboBoxStatisticsTypeGroup->addItem(GET_TEXT("OCCUPANCY/74211", "Sum"), PeopleAll);

    //By Region
    ui->checkBoxGroupChannelRegion->setCountFromChannelName(qMsNvr->maxChannel());

    ui->checkBoxGroupRegion->setCount(4, 2);

    ui->comboBoxLengthOfStayRegion->clear();
    ui->comboBoxLengthOfStayRegion->addItem(GET_TEXT("COMMON/1006", "All"), 0);
    ui->comboBoxLengthOfStayRegion->addItem(GET_TEXT("REGIONAL_PEOPLECOUNTING/103309", "More than"), 1);
    ui->comboBoxLengthOfStayRegion->addItem(GET_TEXT("REGIONAL_PEOPLECOUNTING/103310", "Less than"), 2);

    ui->comboBoxLengthOfStayValueRegion->clear();
    ui->comboBoxLengthOfStayValueRegion->addItem("5s", 5);
    ui->comboBoxLengthOfStayValueRegion->addItem("10s", 10);
    ui->comboBoxLengthOfStayValueRegion->addItem("30s", 30);
    ui->comboBoxLengthOfStayValueRegion->addItem("60s", 60);
    ui->comboBoxLengthOfStayValueRegion->addItem("100s", 100);
    ui->comboBoxLengthOfStayValueRegion->addItem("300s", 300);
    ui->comboBoxLengthOfStayValueRegion->addItem("600s", 600);
    ui->comboBoxLengthOfStayValueRegion->addItem("1800s", 1800);
    ui->comboBoxLengthOfStayValueRegion->setCurrentIndexFromData(60);

    ui->comboBoxReportTypeRegion->clear();
    ui->comboBoxReportTypeRegion->addItem(GET_TEXT("OCCUPANCY/74205", "Daily Report"), DailyReport);
    ui->comboBoxReportTypeRegion->addItem(GET_TEXT("OCCUPANCY/74206", "Weekly Report"), WeeklyReport);
    ui->comboBoxReportTypeRegion->addItem(GET_TEXT("OCCUPANCY/74207", "Monthly Report"), MonthlyReport);

    onLanguageChanged();
}

PagePeopleCountingSearch::~PagePeopleCountingSearch()
{
    delete ui;
}

void PagePeopleCountingSearch::initializeData()
{
    ui->stackedWidget->setCurrentWidget(ui->page_search);

    ui->comboBoxSearchType->setCurrentIndexFromData(PEOPLECNT_SEARCH_BY_CAMERA);

    ui->comboBoxReportTypeCamera->setCurrentIndexFromData(DailyReport);
    ui->comboBoxStatisticsTypeCamera->setCurrentIndexFromData(PeopleEntered);
    ui->dateEditStartCamera->setDate(QDate::currentDate());
    ui->timeEditStartCamera->setTime(QTime(0, 0));

    if (MAX_PEOPLECNT_GROUP > 6) {
        ui->checkBoxGroupGroup->setCountFromGroupName(MAX_PEOPLECNT_GROUP, 3);
    } else {
        ui->checkBoxGroupGroup->setCountFromGroupName(MAX_PEOPLECNT_GROUP, MAX_PEOPLECNT_GROUP);
    }
    ui->comboBoxReportTypeGroup->setCurrentIndexFromData(DailyReport);
    ui->comboBoxStatisticsTypeGroup->setCurrentIndexFromData(PeopleEntered);
    ui->dateEditStartGroup->setDate(QDate::currentDate());
    ui->timeEditStartGroup->setTime(QTime(0, 0));

    ui->checkBoxGroupRegion->clearCheck();
    ui->comboBoxLengthOfStayRegion->setCurrentIndexFromData(0);
    ui->comboBoxReportTypeRegion->setCurrentIndexFromData(DailyReport);
    ui->dateEditStartRegion->setDate(QDate::currentDate());
    ui->timeEditStartRegion->setTime(QTime(0, 0));

    ui->checkBoxGroupChannelCamera->clearCheck();
    ui->checkBoxGroupChannelRegion->clearCheck();
    ui->widgetLineType->clearCheck();
    ui->comboBoxLengthOfStayValueRegion->setCurrentIndexFromData(60);
}

void PagePeopleCountingSearch::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_PEOPLECNT_CACHETODB:
        ON_RESPONSE_FLAG_SET_PEOPLECNT_CACHETODB(message);
        break;
    case RESPONSE_FLAG_GET_IPC_PEOPLE_REPORT:
        ON_RESPONSE_FLAG_GET_IPC_PEOPLE_REPORT(message);
        break;
    default:
        break;
    }
}

void PagePeopleCountingSearch::ON_RESPONSE_FLAG_SET_PEOPLECNT_CACHETODB(MessageReceive *message)
{
    int result = *((int *)message->data);
    qMsDebug() << "result:" << result;
    m_eventLoop.exit();
}

void PagePeopleCountingSearch::ON_RESPONSE_FLAG_GET_IPC_PEOPLE_REPORT(MessageReceive *message)
{
    QString text;
#if 1
    text = QString((char *)message->data);

    qMsDebug() << "text:" << qPrintable(text);
#else
    Q_UNUSED(message)
    switch (gPeopleCountingData.searchType()) {
    case PEOPLECNT_SEARCH_BY_CAMERA:
        text = R"({"incount":"0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,210,220,230","outcount":"230,220,210,200,190,180,170,160,150,140,130,120,110,100,90,80,70,60,50,40,30,20,10,0","starttime":"2021/07/15 00:00:00","endtime":"2021/07/15 23:59:59"})";
        break;
    case PEOPLECNT_SEARCH_BY_REGION:
        text = R"({"incount0":"10,0,20,0,30,0,40,0,50,0,60,0,70,0,80,0,90,0,100,0,110,0,120,0","incount1":"120,0,110,0,100,0,90,0,80,0,70,0,60,0,50,0,40,0,30,0,20,0,10,0","incount2":"0,10,0,20,0,30,0,40,0,50,0,60,0,70,0,80,0,90,0,100,0,110,0,120","incount3":"0,120,0,110,0,100,0,90,0,80,0,70,0,60,0,50,0,40,0,30,0,20,0,10","starttime":"2021/07/15 00:00:00","endtime":"2021/07/15 23:59:59"})";
        break;
    default:
        break;
    }
#endif
    m_textMap.insert(m_currentChannel, text);
    //gPeopleCountingData.dealData(text);
    gEventLoopExit(0);
}

bool PagePeopleCountingSearch::isGroupIncludeMoreChannels()
{
    if (m_searchType != PEOPLECNT_SEARCH_BY_GROUP) {
        return false;
    }

    people_cnt_info *settings_info = gPeopleCountingData.peopleInfo();

    QList<int> checkedList = ui->checkBoxGroupGroup->checkedList();
    for (int i = 0; i < checkedList.size(); ++i) {
        int group_id = checkedList.at(i);
        QString text(settings_info->sets[group_id].tri_channels);
        int count = 0;
        for (int j = 0; j < text.size(); ++j) {
            if (text.at(j) == QChar('1')) {
                count++;
                if (count >= qMsNvr->maxChannel()) {
                    break;
                }
            }
        }
        if (count > MAX_CHECKED) {
            return true;
        }
    }
    return false;
}

void PagePeopleCountingSearch::showSearchResult()
{
    QLayoutItem *item = ui->gridLayoutResult->itemAtPosition(0, 0);
    if (item) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->hide();
        }
        ui->gridLayoutResult->removeItem(item);
        delete item;
    }
    //
    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_GROUP:
        if (!m_groupResult) {
            m_groupResult = new PeopleCountingGroupResult(this);
        }
        ui->gridLayoutResult->addWidget(m_groupResult, 0, 0);
        m_groupResult->showGroupResult(m_checkedList);
        break;
    case PEOPLECNT_SEARCH_BY_CAMERA:
        if (!m_cameraResult) {
            m_cameraResult = new PeopleCountingCameraResult(this);
        }
        ui->gridLayoutResult->addWidget(m_cameraResult, 0, 0);
        m_cameraResult->setMainTitle(GET_TEXT("PEOPLECOUNTING_SEARCH/140000", "People Counting by Camera"));
        m_cameraResult->setLineTab(ui->widgetLineType->checkedFlags());
        m_cameraResult->showCameraResult(m_checkedList, m_checkedCameraList, m_textMap);
        break;
    case PEOPLECNT_SEARCH_BY_REGION:
        if (!m_regionResult) {
            m_regionResult = new PeopleCountingRegionResult(this);
        }
        ui->gridLayoutResult->addWidget(m_regionResult, 0, 0);
        m_regionResult->setMainTitle(GET_TEXT("PEOPLECOUNTING_SEARCH/140002", "Regional People Counting"));
        m_regionResult->showRegionResult(m_checkedList, m_checkedCameraList, m_textMap);
        break;
    default:
        break;
    }

    ui->stackedWidget->setCurrentWidget(ui->page_result);
}

void PagePeopleCountingSearch::onLanguageChanged()
{
    ui->labelSearchType->setText(GET_TEXT("OCCUPANCY/74201", "Search Type"));

    ui->labelChannelCamera->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelReportTypeCamera->setText(GET_TEXT("OCCUPANCY/74204", "Report Type"));
    ui->labelStatisticsTypeCamera->setText(GET_TEXT("OCCUPANCY/74208", "Statistic Type"));
    ui->labelStartTimeCamera->setText(GET_TEXT("OCCUPANCY/74212", "Start Time"));

    ui->labelGroupGroup->setText(GET_TEXT("OCCUPANCY/74203", "Group"));
    ui->labelReportTypeGroup->setText(GET_TEXT("OCCUPANCY/74204", "Report Type"));
    ui->labelStatisticsTypeGroup->setText(GET_TEXT("OCCUPANCY/74208", "Statistic Type"));
    ui->labelStartTimeGroup->setText(GET_TEXT("OCCUPANCY/74212", "Start Time"));

    ui->labelRegionRegion->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/140003", "Region"));
    ui->labelChannelRegion->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelLengthOfStayRegion->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/103302", "Length of Stay"));
    ui->labelReportTypeRegion->setText(GET_TEXT("OCCUPANCY/74204", "Report Type"));
    ui->labelStartTimeRegion->setText(GET_TEXT("OCCUPANCY/74212", "Start Time"));

    ui->pushButtonSearch->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->pushButtonBackupAll->setText(GET_TEXT("COMMONBACKUP/100002", "Backup All"));
    ui->pushButtonBackup->setText(GET_TEXT("PROFILE/76001", "Backup"));
    ui->pushButtonResultBack->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->labelLine->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line"));
}

void PagePeopleCountingSearch::on_comboBoxSearchType_indexSet(int index)
{
    m_searchType = static_cast<PEOPLECNT_SEARCH_TYPE_E>(ui->comboBoxSearchType->itemData(index).toInt());
    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_CAMERA:
        ui->stackedWidgetSearchType->setCurrentWidget(ui->pageCamera);
        break;
    case PEOPLECNT_SEARCH_BY_GROUP:
        ui->stackedWidgetSearchType->setCurrentWidget(ui->pageGroup);
        break;
    case PEOPLECNT_SEARCH_BY_REGION:
        ui->stackedWidgetSearchType->setCurrentWidget(ui->pageRegion);
        break;
    default:
        break;
    }
}

void PagePeopleCountingSearch::on_comboBoxLengthOfStayRegion_indexSet(int index)
{
    int value = ui->comboBoxLengthOfStayRegion->itemData(index).toInt();
    switch (value) {
    case 0:
        ui->comboBoxLengthOfStayValueRegion->hide();
        break;
    default:
        ui->comboBoxLengthOfStayValueRegion->show();
        break;
    }
}

void PagePeopleCountingSearch::on_pushButtonSearch_clicked()
{
    ui->pushButtonSearch->clearUnderMouse();
    ui->pushButtonSearch->clearFocus();
    return;

    m_checkedList.clear();
    m_textMap.clear();
    gPeopleCountingData.clearCameraMap();

    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_GROUP: {
        QList<int> checkedList = ui->checkBoxGroupGroup->checkedList();
        if (checkedList.isEmpty()) {
            ShowMessageBox(GET_TEXT("OCCUPANCY/74239", "Please select at least one group."));
            return;
        }
        if (checkedList.size() > MAX_CHECKED) {
            ExecMessageBox(GET_TEXT("OCCUPANCY/74242", "Over 10 channels. Only the total data will be showed."));
        } else if (isGroupIncludeMoreChannels()) {
            ExecMessageBox(GET_TEXT("OCCUPANCY/74240", "Group includes more than 10 channels. Only the total data will be showed."));
        }

        m_checkedList = ui->checkBoxGroupGroup->checkedList();
        m_recordType = static_cast<ReportType>(ui->comboBoxReportTypeGroup->currentIntData());
        m_statisticsType = static_cast<StatisticsType>(ui->comboBoxStatisticsTypeGroup->currentIntData());
        QDateTime startDateTime = QDateTime(ui->dateEditStartGroup->date(), ui->timeEditStartGroup->time());
        gPeopleCountingData.setSearchInfo(m_searchType, m_recordType, m_statisticsType, startDateTime, m_checkedList);

        //
        //MsWaitting::showGlobalWait();
        sendMessage(REQUEST_FLAG_SET_PEOPLECNT_CACHETODB, nullptr, 0);
        //m_eventLoop.exec();
        //gPeopleCountingData.waitForSearchData();
        //MsWaitting::closeGlobalWait();
        break;
    }
    case PEOPLECNT_SEARCH_BY_CAMERA: {
        QList<int> checkedList = ui->checkBoxGroupChannelCamera->checkedList();
        if (checkedList.isEmpty()) {
            ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel."));
            return;
        }
        switch (ui->comboBoxStatisticsTypeCamera->currentIntData()) {
        case PeopleEntered:
            //只显示People Entered
            m_checkedList << 0;
            break;
        case PeopleExited:
            //只显示People Existed
            m_checkedList << 1;
            break;
        case PeopleAll:
            //显示Sum People Entered People Existed
            m_checkedList << -1 << 0 << 1;
            break;
        }
        m_checkedCameraList = ui->checkBoxGroupChannelCamera->checkedList();
        m_recordType = static_cast<ReportType>(ui->comboBoxReportTypeCamera->currentIntData());
        m_statisticsType = static_cast<StatisticsType>(ui->comboBoxStatisticsTypeCamera->currentIntData());
        QDateTime startDateTime = QDateTime(ui->dateEditStartCamera->date(), ui->timeEditStartCamera->time());
        gPeopleCountingData.setSearchInfo(m_searchType, m_recordType, m_statisticsType, startDateTime, m_checkedList);
        //gPeopleCountingData.setCurrentChannel(channel);

        //MsWaitting::showGlobalWait();
        ReqIpcPeopleReport report;
        report.mainType = 0;
        report.reportType = m_recordType;
        //选择total，需要获得四条线的数据
        if (ui->widgetLineType->checkedFlags() == 0) {
            ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145023", "Please select at least one line."));
            //MsWaitting::closeGlobalWait();
            return;
        } else {
            report.lineMask = 15;
        }
        snprintf(report.startTime, sizeof(report.startTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toLocal8Bit().data());
        for (auto channel : m_checkedCameraList) {
            m_currentChannel = channel;
            report.chnid = channel;
            sendMessage(REQUEST_FLAG_GET_IPC_PEOPLE_REPORT, &report, sizeof(report));
            gEventLoopExec();
        }
        //MsWaitting::closeGlobalWait();
        break;
    }
    case PEOPLECNT_SEARCH_BY_REGION: {
        QList<int> checkedList = ui->checkBoxGroupChannelRegion->checkedList();
        if (checkedList.isEmpty()) {
            ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel."));
            return;
        }
        m_checkedList = ui->checkBoxGroupRegion->checkedList();
        if (m_checkedList.isEmpty()) {
            MessageBox::information(this, GET_TEXT("REGIONAL_PEOPLECOUNTING/103107", "Please select at least one region."));
            return;
        }
        if (m_checkedList.size() > 1) {
            m_checkedList.prepend(-1);
        }

        m_checkedCameraList = ui->checkBoxGroupChannelRegion->checkedList();
        m_recordType = static_cast<ReportType>(ui->comboBoxReportTypeRegion->currentIntData());
        QDateTime startDateTime = QDateTime(ui->dateEditStartRegion->date(), ui->timeEditStartRegion->time());
        gPeopleCountingData.setSearchInfo(m_searchType, m_recordType, StatisticsNone, startDateTime, m_checkedList);
        //gPeopleCountingData.setCurrentChannel(channel);

        //MsWaitting::showGlobalWait();
        ReqIpcPeopleReport report;

        report.mainType = 1;
        report.reportType = m_recordType;
        snprintf(report.startTime, sizeof(report.startTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toLocal8Bit().data());
        report.region = ui->checkBoxGroupRegion->checkedFlags();
        report.lengthOfStayType = ui->comboBoxLengthOfStayRegion->currentIntData();
        report.lengthOfStayValue = ui->comboBoxLengthOfStayValueRegion->currentIntData();
        for (auto channel : m_checkedCameraList) {
            m_currentChannel = channel;
            report.chnid = channel;
            sendMessage(REQUEST_FLAG_GET_IPC_PEOPLE_REPORT, &report, sizeof(report));
            gEventLoopExec();
        }
        //MsWaitting::closeGlobalWait();
        break;
    }
    default:
        break;
    }

    //
    showSearchResult();
}

void PagePeopleCountingSearch::on_pushButtonBack_clicked()
{
    ui->pushButtonBack->clearFocus();
    back();
}

void PagePeopleCountingSearch::on_pushButtonBackupAll_clicked()
{
    ui->pushButtonBackupAll->clearUnderMouse();
    ui->pushButtonBackupAll->clearFocus();
    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_GROUP:
        if (m_groupResult) {
            m_groupResult->backupAll();
        }
        break;
    case PEOPLECNT_SEARCH_BY_CAMERA:
        if (m_cameraResult) {
            m_cameraResult->backupAll();
        }
        break;
    case PEOPLECNT_SEARCH_BY_REGION:
        if (m_regionResult) {
            m_regionResult->backupAll();
        }
        break;
    default:
        break;
    }
}

void PagePeopleCountingSearch::on_pushButtonBackup_clicked()
{
    ui->pushButtonBackup->clearUnderMouse();
    ui->pushButtonBackup->clearFocus();
    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_GROUP:
        if (m_groupResult) {
            m_groupResult->backup();
        }
        break;
    case PEOPLECNT_SEARCH_BY_CAMERA:
        if (m_cameraResult) {
            m_cameraResult->backup();
        }
        break;
    case PEOPLECNT_SEARCH_BY_REGION:
        if (m_regionResult) {
            m_regionResult->backup();
        }
        break;
    default:
        break;
    }
}

void PagePeopleCountingSearch::on_pushButtonResultBack_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_search);
}
