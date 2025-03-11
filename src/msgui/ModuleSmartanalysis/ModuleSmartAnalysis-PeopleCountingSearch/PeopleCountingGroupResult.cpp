#include "PeopleCountingGroupResult.h"
#include "ui_PeopleCountingGroupResult.h"
#include "HistogramScene.h"
#include "LineScene.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyButtonGroup.h"
#include "MyDebug.h"
#include <QToolButton>

PeopleCountingGroupResult::PeopleCountingGroupResult(QWidget *parent)
    : AbstractPeopleCountingResult(parent)
    , ui(new Ui::PeopleCountingGroupResult)
{
    ui->setupUi(this);

    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)), Qt::QueuedConnection);

    m_lineScene = new LineScene(this);
    m_histogramScene = new HistogramScene(this);

    m_toolButtonPolyline->raise();
    m_toolButtonHistogram->raise();
}

PeopleCountingGroupResult::~PeopleCountingGroupResult()
{
    delete ui;
}

void PeopleCountingGroupResult::setMainTitle(const QString &text)
{
    m_mainTitle = text;
    m_lineScene->setMainTitle(text);
    m_histogramScene->setMainTitle(text);
}

void PeopleCountingGroupResult::setSubTitle(const QString &text)
{
    m_subTitle = text;
    m_lineScene->setSubTitle(text);
    m_histogramScene->setSubTitle(text);
}

void PeopleCountingGroupResult::showGroupResult(const QList<int> &groupList)
{
    show();

    m_lineScene->clearChannelVisible();
    m_histogramScene->clearChannelVisible();
    m_groupList = groupList;
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
    ui->tabBar->clear();
    people_cnt_info *people_info = gPeopleCountingData.peopleInfo();
    for (int i = 0; i < m_groupList.size(); ++i) {
        int group = m_groupList.at(i);
        const peoplecnt_setting &setting = people_info->sets[group];
        QString groupName(setting.name);
        if (groupName.isEmpty()) {
            groupName = QString("Group%1").arg(i + 1);
        }
        ui->tabBar->addTab(groupName, group);
    }

    //
    ui->radioButtonHistogram->setChecked(true);
    m_buttonGroup->setCurrentId(ButtonPolyline);

    //slot: onTabClicked
    ui->tabBar->setCurrentTab(m_groupList.first());
}

void PeopleCountingGroupResult::backup()
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
    QList<int> groupList;
    groupList << m_currentGroup;
    //
    AbstractPeopleCountingResult::backupGroup(ui->graphicsView->viewport()->geometry(), mode, groupList);
}

void PeopleCountingGroupResult::backupAll()
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
    AbstractPeopleCountingResult::backupGroup(ui->graphicsView->viewport()->geometry(), mode, m_groupList);
}

void PeopleCountingGroupResult::onTabClicked(int index)
{
    m_currentGroup = index;

    m_channelList.clear();
    m_channelList.append(TOTAL_LINE_INDEX);
    const PEOPLECNT_SETTING &setting = gPeopleCountingData.peopleInfo()->sets[m_currentGroup];
    QString checkedMask(setting.tri_channels);
    for (int i = 0; i < checkedMask.size(); ++i) {
        if (checkedMask.at(i) == QChar('1') && i < qMsNvr->maxChannel()) {
            m_channelList.append(i);
        }
    }

    m_lineScene->setCurrentGroup(m_currentGroup);
    m_histogramScene->setCurrentGroup(m_currentGroup);
    if (m_toolButtonPolyline->isChecked()) {
        showLine();
    } else {
        if (ui->radioButtonHistogram->isChecked()) {
            showHistogram();
        } else {
            showHistogram2();
        }
    }

    //
    setMainTitle(GET_TEXT("PEOPLECOUNTING_SEARCH/140001", "People Counting by Group"));
    QString sub = QString("%1 - %2 - %3 - ")
                      .arg(gPeopleCountingData.startDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                      .arg(gPeopleCountingData.endDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                      .arg(gPeopleCountingData.statisticsTypeString());
    setSubTitle(sub + ui->tabBar->tabText(index));
    setSubTitleForBackup(sub);
}

void PeopleCountingGroupResult::onPolylineClicked()
{
    QMetaObject::invokeMethod(this, "showLine", Qt::QueuedConnection);
}

void PeopleCountingGroupResult::onHistogramClicked()
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

void PeopleCountingGroupResult::showLine()
{
    ui->graphicsView->setScene(m_lineScene);
    m_lineScene->setSceneRect(ui->graphicsView->viewport()->geometry());
    m_lineScene->showLine(m_channelList, m_currentGroup);

    ui->radioButtonHistogram->hide();
    ui->radioButtonHistogram2->hide();
}

void PeopleCountingGroupResult::showHistogram()
{
    ui->graphicsView->setScene(m_histogramScene);
    m_histogramScene->setSceneRect(ui->graphicsView->viewport()->geometry());
    m_histogramScene->showHistogram(m_channelList, m_currentGroup);

    ui->radioButtonHistogram->show();
    ui->radioButtonHistogram2->show();
}

void PeopleCountingGroupResult::showHistogram2()
{
    ui->graphicsView->setScene(m_histogramScene);
    m_histogramScene->setSceneRect(ui->graphicsView->viewport()->geometry());
    m_histogramScene->showHistogram2(m_channelList, m_currentGroup);

    ui->radioButtonHistogram->show();
    ui->radioButtonHistogram2->show();
}

void PeopleCountingGroupResult::on_radioButtonHistogram_clicked(bool checked)
{
    if (checked) {
        showHistogram();
    }
}

void PeopleCountingGroupResult::on_radioButtonHistogram2_clicked(bool checked)
{
    if (checked) {
        showHistogram2();
    }
}
