#include "PeopleCountingCameraResult.h"
#include "ui_PeopleCountingCameraResult.h"
#include "CameraPeopleCountingHistogramScene.h"
#include "CameraPeopleCountingPolylineScene.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyButtonGroup.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include <QToolButton>

PeopleCountingCameraResult::PeopleCountingCameraResult(QWidget *parent)
    : AbstractPeopleCountingResult(parent)
    , ui(new Ui::PeopleCountingCameraResult)
{
    ui->setupUi(this);
    ui->tabBar->setHSpacing(40);
    ui->tabBar->setLeftMargin(0);

    m_lineScene = new CameraPeopleCountingPolylineScene(this);
    m_histogramScene = new CameraPeopleCountingHistogramScene(this);

    m_toolButtonPolyline->raise();
    m_toolButtonHistogram->raise();
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabBarClicked(int)));

    connect(this, SIGNAL(backupFinished(int)), this, SLOT(on_comboBoxChannel_activated(int)));
    onLanguageChanged();
}

PeopleCountingCameraResult::~PeopleCountingCameraResult()
{
    delete ui;
}

void PeopleCountingCameraResult::setMainTitle(const QString &text)
{
    m_mainTitle = text;
    m_lineScene->setMainTitle(text);
    m_histogramScene->setMainTitle(text);
}

void PeopleCountingCameraResult::setSubTitle(const QString &text)
{
    m_subTitle = text;
    m_lineScene->setSubTitle(text);
    m_histogramScene->setSubTitle(text);
}

void PeopleCountingCameraResult::setLineTab(quint64 lineMask)
{
    m_lineMask = static_cast<int>(lineMask);
    ui->tabBar->clear();
    if (lineMask & 1) {
        ui->tabBar->addTab(GET_TEXT("PEOPLECOUNTING_SEARCH/145022", "Total"), 0);
    }
    if (lineMask & (1 << 1)) {
        ui->tabBar->addTab(GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 1", 1);
    }
    if (lineMask & (1 << 2)) {
        ui->tabBar->addTab(GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 2", 2);
    }
    if (lineMask & (1 << 3)) {
        ui->tabBar->addTab(GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 3", 3);
    }
    if (lineMask & (1 << 4)) {
        ui->tabBar->addTab(GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 4", 4);
    }
}

void PeopleCountingCameraResult::showCameraResult(const QList<int> &cameraList, const QList<int> checkedCameraList, const QMap<int, QString> textMap)
{
    show();
    //
    m_lineScene->clearChannelVisible();
    m_histogramScene->clearChannelVisible();
    m_channelList = cameraList;
    m_checkedCameraList = checkedCameraList;
    m_textMap = textMap;

    ui->comboBoxChannel->clear();
    for (auto channel : m_checkedCameraList) {
        ui->comboBoxChannel->addItem(QString("%1").arg(channel + 1), channel);
    }
    on_comboBoxChannel_activated(0);
    //
    ReportType report = gPeopleCountingData.reportType();
    switch (report) {
    case DailyReport:
        ui->radioButtonHistogram->setText(GET_TEXT("OCCUPANCY/74256", "Daily Report by Hour"));
        ui->radioButtonHistogram2->setText(GET_TEXT("OCCUPANCY/74257", "Daily Report"));
        break;
    case WeeklyReport:
        ui->radioButtonHistogram->setText(GET_TEXT("OCCUPANCY/74258", "Weekly Report by Day"));
        ui->radioButtonHistogram2->setText(GET_TEXT("OCCUPANCY/74259", "Weekly Report"));
        break;
    case MonthlyReport:
        ui->radioButtonHistogram->setText(GET_TEXT("OCCUPANCY/74260", "Monthly Report by Day"));
        ui->radioButtonHistogram2->setText(GET_TEXT("OCCUPANCY/74261", "Monthly Report"));
        break;
    default:
        qMsWarning() << "invalid report type:" << report;
        break;
    }
    //
    ui->radioButtonHistogram->setChecked(true);
    m_buttonGroup->setCurrentId(ButtonPolyline);
}

void PeopleCountingCameraResult::backup()
{
    ChartMode mode;
    if (m_toolButtonPolyline->isChecked()) {
        mode = LineChart;
    } else {
        if (ui->radioButtonHistogram->isChecked()) {
            mode = HistogramChart;
        } else {
            mode = Histogram2Chart;
        }
    }
    //
    QList<int> channels;
    channels << ui->comboBoxChannel->currentIntData();
    AbstractPeopleCountingResult::backupCamera(ui->graphicsView->viewport()->geometry(), mode, channels, m_textMap, m_lineMask);
}

void PeopleCountingCameraResult::backupAll()
{
    ChartMode mode;
    if (m_toolButtonPolyline->isChecked()) {
        mode = LineChart;
    } else {
        if (ui->radioButtonHistogram->isChecked()) {
            mode = HistogramChart;
        } else {
            mode = Histogram2Chart;
        }
    }
    //
    AbstractPeopleCountingResult::backupCamera(ui->graphicsView->viewport()->geometry(), mode, m_checkedCameraList, m_textMap, m_lineMask);
}

void PeopleCountingCameraResult::onPolylineClicked()
{
    QMetaObject::invokeMethod(this, "showLine", Qt::QueuedConnection);
}

void PeopleCountingCameraResult::onHistogramClicked()
{
    if (ui->radioButtonHistogram->isChecked()) {
        QMetaObject::invokeMethod(this, "showHistogram", Qt::QueuedConnection);
        return;
    }
    if (ui->radioButtonHistogram2->isChecked()) {
        QMetaObject::invokeMethod(this, "showHistogram2", Qt::QueuedConnection);
        return;
    }
}

void PeopleCountingCameraResult::onLanguageChanged()
{
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    AbstractPeopleCountingResult::onLanguageChanged();
}

void PeopleCountingCameraResult::showLine()
{
    ui->graphicsView->setScene(m_lineScene);
    m_lineScene->setSceneRect(ui->graphicsView->viewport()->geometry());
    m_lineScene->showLine(m_channelList);

    ui->radioButtonHistogram->hide();
    ui->radioButtonHistogram2->hide();
}

void PeopleCountingCameraResult::showHistogram()
{
    ui->graphicsView->setScene(m_histogramScene);
    m_histogramScene->setSceneRect(ui->graphicsView->viewport()->geometry());
    m_histogramScene->showHistogram(m_channelList);

    ui->radioButtonHistogram->show();
    ui->radioButtonHistogram2->show();
}

void PeopleCountingCameraResult::showHistogram2()
{
    ui->graphicsView->setScene(m_histogramScene);
    m_histogramScene->setSceneRect(ui->graphicsView->viewport()->geometry());
    m_histogramScene->showHistogram2(m_channelList);

    ui->radioButtonHistogram->show();
    ui->radioButtonHistogram2->show();
}

void PeopleCountingCameraResult::on_radioButtonHistogram_clicked(bool checked)
{
    if (checked) {
        showHistogram();
    }
}

void PeopleCountingCameraResult::on_radioButtonHistogram2_clicked(bool checked)
{
    if (checked) {
        showHistogram2();
    }
}

void PeopleCountingCameraResult::on_comboBoxChannel_activated(int index)
{
    Q_UNUSED(index);
    int channel = ui->comboBoxChannel->currentIntData();
    gPeopleCountingData.setCurrentChannel(channel);
    if (ui->tabBar->currentTab() != -1) {
        onTabBarClicked(ui->tabBar->currentTab());
    } else {
        ui->tabBar->setFirstTab();
    }

}

void PeopleCountingCameraResult::onTabBarClicked(int index)
{
    int channel = ui->comboBoxChannel->currentIntData();
    setSubTitle(QString("%1 - %2 - %3 - %4 - %5")
                    .arg(gPeopleCountingData.startDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                    .arg(gPeopleCountingData.endDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                    .arg(gPeopleCountingData.statisticsTypeString())
                    .arg(qMsNvr->channelName(channel))
                    .arg(ui->tabBar->tabText(ui->tabBar->currentTab())));
    gPeopleCountingData.dealData(m_textMap[channel], index);
    if (m_toolButtonPolyline->isChecked()) {
        onPolylineClicked();
    } else {
        onHistogramClicked();
    }
}
