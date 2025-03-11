#include "PeopleCountingRegionResult.h"
#include "ui_PeopleCountingRegionResult.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyButtonGroup.h"
#include "MyDebug.h"
#include "RegionPeopleCountingHistogramScene.h"
#include "RegionPeopleCountingPolylineScene.h"
#include <QToolButton>

PeopleCountingRegionResult::PeopleCountingRegionResult(QWidget *parent)
    : AbstractPeopleCountingResult(parent)
    , ui(new Ui::PeopleCountingRegionResult)
{
    ui->setupUi(this);

    m_lineScene = new RegionPeopleCountingPolylineScene(this);
    m_histogramScene = new RegionPeopleCountingHistogramScene(this);

    m_toolButtonPolyline->raise();
    m_toolButtonHistogram->raise();
    connect(this, SIGNAL(backupFinished(int)), this, SLOT(on_comboBoxChannel_activated(int)));
}

PeopleCountingRegionResult::~PeopleCountingRegionResult()
{
    delete ui;
}

void PeopleCountingRegionResult::setMainTitle(const QString &text)
{
    m_mainTitle = text;
    m_lineScene->setMainTitle(text);
    m_histogramScene->setMainTitle(text);
}

void PeopleCountingRegionResult::setSubTitle(const QString &text)
{
    m_subTitle = text;
    m_lineScene->setSubTitle(text);
    m_histogramScene->setSubTitle(text);
}

void PeopleCountingRegionResult::showRegionResult(const QList<int> &cameraList, const QList<int> checkedCameraList, const QMap<int, QString> textMap)
{
    show();
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
    m_lineScene->clearChannelVisible();
    m_histogramScene->clearChannelVisible();
    m_channelList = cameraList;
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

void PeopleCountingRegionResult::backup()
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
    QList <int> channels;
    channels << ui->comboBoxChannel->currentIntData();
    AbstractPeopleCountingResult::backupRegions(ui->graphicsView->viewport()->geometry(), mode, channels);
}

void PeopleCountingRegionResult::backupAll()
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
    //绘制所有没有绘制过的加入缓存
    for (auto channel : m_checkedCameraList) {
        if (!gPeopleCountingData.isHasMapKey(channel)) {
            gPeopleCountingData.setCurrentChannel(channel);
            gPeopleCountingData.dealData(m_textMap[channel]);
        }
    }
    //
    AbstractPeopleCountingResult::backupRegions(ui->graphicsView->viewport()->geometry(), mode, m_checkedCameraList);
}

void PeopleCountingRegionResult::onPolylineClicked()
{
    QMetaObject::invokeMethod(this, "showLine", Qt::QueuedConnection);
}

void PeopleCountingRegionResult::onHistogramClicked()
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

void PeopleCountingRegionResult::onLanguageChanged()
{
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    AbstractPeopleCountingResult::onLanguageChanged();
}

void PeopleCountingRegionResult::showLine()
{
    ui->graphicsView->setScene(m_lineScene);
    m_lineScene->setSceneRect(ui->graphicsView->viewport()->geometry());
    m_lineScene->showLine(m_channelList);

    ui->radioButtonHistogram->hide();
    ui->radioButtonHistogram2->hide();
}

void PeopleCountingRegionResult::showHistogram()
{
    ui->graphicsView->setScene(m_histogramScene);
    m_histogramScene->setSceneRect(ui->graphicsView->viewport()->geometry());
    m_histogramScene->showHistogram(m_channelList);

    ui->radioButtonHistogram->show();
    ui->radioButtonHistogram2->show();
}

void PeopleCountingRegionResult::showHistogram2()
{
    ui->graphicsView->setScene(m_histogramScene);
    m_histogramScene->setSceneRect(ui->graphicsView->viewport()->geometry());
    m_histogramScene->showHistogram2(m_channelList);

    ui->radioButtonHistogram->show();
    ui->radioButtonHistogram2->show();
}

void PeopleCountingRegionResult::on_radioButtonHistogram_clicked(bool checked)
{
    if (checked) {
        showHistogram();
    }
}

void PeopleCountingRegionResult::on_radioButtonHistogram2_clicked(bool checked)
{
    if (checked) {
        showHistogram2();
    }
}

void PeopleCountingRegionResult::on_comboBoxChannel_activated(int index)
{
    Q_UNUSED(index);
    int channel = ui->comboBoxChannel->currentIntData();
    setSubTitle(QString("%1 - %2 - %3")
                    .arg(gPeopleCountingData.startDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                    .arg(gPeopleCountingData.endDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                    .arg(qMsNvr->channelName(channel)));
    gPeopleCountingData.setCurrentChannel(channel);
    gPeopleCountingData.dealData(m_textMap[channel]);
    if (m_toolButtonPolyline->isChecked()) {
        onPolylineClicked();
    } else {
        onHistogramClicked();
    }
}
