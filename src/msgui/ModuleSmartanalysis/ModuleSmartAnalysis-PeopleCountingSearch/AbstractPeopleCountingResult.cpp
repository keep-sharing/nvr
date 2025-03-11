#include "AbstractPeopleCountingResult.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyButtonGroup.h"
#include "MyDebug.h"
#include "PeopleCountingBackup.h"
#include "MyFileSystemDialog.h"
#include <QToolButton>

AbstractPeopleCountingResult::AbstractPeopleCountingResult(QWidget *parent)
    : QWidget(parent)
{

    m_toolButtonPolyline = new QToolButton(this);
    m_toolButtonPolyline->setObjectName("toolButtonPolyline");
    m_toolButtonPolyline->setStyleSheet("QToolButton{border:0px;padding:2px;}QToolButton:checked{border:0px;}");
    m_toolButtonPolyline->setIconSize(QSize(26, 26));
    m_toolButtonPolyline->setCheckable(true);
    m_toolButtonPolyline->setChecked(true);

    m_toolButtonHistogram = new QToolButton(this);
    m_toolButtonHistogram->setObjectName("toolButtonHistogram");
    m_toolButtonHistogram->setStyleSheet("QToolButton{border:0px;padding:2px;}QToolButton:checked{border:0px;}");
    m_toolButtonHistogram->setIconSize(QSize(26, 26));
    m_toolButtonHistogram->setCheckable(true);

    m_buttonGroup = new MyButtonGroup(this);
    m_buttonGroup->addButton(m_toolButtonPolyline, ButtonPolyline);
    m_buttonGroup->addButton(m_toolButtonHistogram, ButtonHistogram);
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroupClicked(int)));

    m_backup = new PeopleCountingBackup(this);
    connect(m_backup, SIGNAL(backupFinished(int)), this, SLOT(onBackupFinished(int)));

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    QMetaObject::invokeMethod(this, "onLanguageChanged", Qt::QueuedConnection);
}

AbstractPeopleCountingResult::~AbstractPeopleCountingResult()
{
}

void AbstractPeopleCountingResult::backupCamera(const QRect &rc, int mode, const QList<int> &channels, const QMap<int, QString> &textMap, const int lineMask)
{
    ExportPeopleCountingInfo info = MyFileSystemDialog::instance()->exportPeopleCounting();
    if (!info.isValid()) {
        return;
    }

    //info.path = "/root";

    //MsWaitting::showGlobalWait();
    //
    m_backup->setSelectTypes(m_channelList);
    m_backup->setChannels(channels);
    m_backup->setViewportGeometry(rc);
    m_backup->setChartMode((ChartMode)mode);
    m_backup->setMainTitle(m_mainTitle);
    m_backup->setSubTitle(m_subTitle);
    m_backup->setSubTitleForBackup(m_subTitleForBackup);
    m_backup->setTextMap(textMap);
    m_backup->setLineMask(lineMask);
    m_backup->startBackup(info);
}

void AbstractPeopleCountingResult::backupRegions(const QRect &rc, int mode, const QList<int> &channels)
{
    ExportPeopleCountingInfo info = MyFileSystemDialog::instance()->exportPeopleCounting();
    if (!info.isValid()) {
        return;
    }

    //info.path = "/root";

    //MsWaitting::showGlobalWait();
    //
    m_backup->setSelectTypes(m_channelList);
    m_backup->setChannels(channels);
    m_backup->setViewportGeometry(rc);
    m_backup->setChartMode((ChartMode)mode);
    m_backup->setMainTitle(m_mainTitle);
    m_backup->setSubTitle(m_subTitle);
    m_backup->setSubTitleForBackup(m_subTitleForBackup);
    m_backup->startBackup(info);
}

void AbstractPeopleCountingResult::backupGroup(const QRect &rc, int mode, const QList<int> &groups)
{
    ExportPeopleCountingInfo info = MyFileSystemDialog::instance()->exportPeopleCounting();
    if (!info.isValid()) {
        return;
    }

    //info.path = "/root";

    //MsWaitting::showGlobalWait();
    //
    m_backup->setGroups(groups);
    m_backup->setViewportGeometry(rc);
    m_backup->setChartMode((ChartMode)mode);
    m_backup->setMainTitle(m_mainTitle);
    m_backup->setSubTitle(m_subTitle);
    m_backup->setSubTitleForBackup(m_subTitleForBackup);
    m_backup->startBackup(info);
}

void AbstractPeopleCountingResult::setSubTitleForBackup(const QString &text)
{
    m_subTitleForBackup = text;
}

void AbstractPeopleCountingResult::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    //
    m_toolButtonPolyline->setGeometry(width() - 159, 110, 30, 30);
    m_toolButtonHistogram->setGeometry(width() - 119, 110, 30, 30);
}

void AbstractPeopleCountingResult::onLanguageChanged()
{
    m_toolButtonPolyline->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/140004", "Line Chart"));
    m_toolButtonHistogram->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/140005", "Bar Chart"));
}

void AbstractPeopleCountingResult::onButtonGroupClicked(int id)
{
    switch (id) {
    case ButtonPolyline:
        m_toolButtonPolyline->setIcon(QIcon(":/liveview_occupancy/liveview_occupancy/line_checked.png"));
        m_toolButtonHistogram->setIcon(QIcon(":/liveview_occupancy/liveview_occupancy/histogram.png"));
        onPolylineClicked();
        break;
    case ButtonHistogram:
        m_toolButtonPolyline->setIcon(QIcon(":/liveview_occupancy/liveview_occupancy/line.png"));
        m_toolButtonHistogram->setIcon(QIcon(":/liveview_occupancy/liveview_occupancy/histogram_checked.png"));
        onHistogramClicked();
        break;
    }
}

void AbstractPeopleCountingResult::onBackupFinished(int result)
{
    //MsWaitting::closeGlobalWait();

    if (result == 1) {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145000", "Backup successfully."));
    } else if (result == 3) {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145001", "Failed to backup some results."));
    } else {
        ShowMessageBox(GET_TEXT("PEOPLECOUNTING_SEARCH/145002", "Failed to backup."));
    }
    emit backupFinished(0);
}
