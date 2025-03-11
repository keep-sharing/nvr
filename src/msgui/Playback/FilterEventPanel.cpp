#include "FilterEventPanel.h"
#include "ui_FilterEventPanel.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PlaybackBar.h"
#include "PlaybackEventData.h"
#include "PlaybackMode.h"
#include "PlaybackRealTimeThread.h"
#include "SmartSearchControl.h"
#include "centralmessage.h"
#include <QMouseEvent>

FilterEventPanel::FilterEventPanel(QWidget *parent)
    : BasePlayback(parent)
    , ui(new Ui::FilterEventPanel)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    s_filterEventPanel = this;
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

FilterEventPanel::~FilterEventPanel()
{
#ifdef MS_FISHEYE_SOFT_DEWARP
    s_fisheyeControl = nullptr;
#endif
    delete ui;
}

void FilterEventPanel::initializeData()
{
    if (getFilterEvent() != INFO_MAJOR_NONE) {
        ui->comboBoxFilter->setCurrentIndexFromData(getFilterEvent());
        ui->checkBoxFilter->setChecked(true);
    } else {
        ui->comboBoxFilter->setCurrentIndex(0);
        ui->checkBoxFilter->setChecked(false);
    }
}

void FilterEventPanel::mousePressEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    if (!ui->widget_translucent->geometry().contains(p)) {
        closePanel();
    }
    return QWidget::mousePressEvent(event);
}

void FilterEventPanel::closePanel()
{
    hide();
}

void FilterEventPanel::onFilterEventPanelButtonClicked(int x, int y)
{
    Q_UNUSED(x)
    if (!isVisible()) {
        initializeData();
        show();
        raise();
        QPoint p;
        p.setX(0);
        p.setY(y - height());
        move(p);
    } else {
        closePanel();
    }
}

void FilterEventPanel::onLanguageChanged()
{
    ui->comboBoxFilter->clear();
    ui->comboBoxFilter->addItem(GET_TEXT("MOTION/51000", "Motion Detection"), INFO_MAJOR_MOTION);
    ui->comboBoxFilter->addItem(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"), INFO_MAJOR_AUDIO_ALARM);
    ui->comboBoxFilter->addItem(GET_TEXT("ALARMIN/52001", "Alarm Input"), INFO_MAJOR_ALARMIN);
    ui->comboBoxFilter->addItem(GET_TEXT("SMARTEVENT/55000", "VCA"), INFO_MAJOR_VCA);
    ui->comboBoxFilter->addItem(GET_TEXT("ANPR/103054", "Smart Analysis"), INFO_MAJOR_SMART);

    ui->pushButtonOverrideSearch->setText(GET_TEXT("LIVEVIEW/168006", "Override Search"));
    ui->pushButtonEventOnlySearch->setText(GET_TEXT("LIVEVIEW/168007", "Event Only Search"));
}

void FilterEventPanel::onPushButtonClicked(bool isEventOnly)
{
    if (!ui->checkBoxFilter->isChecked()) {
        return;
    }
    setFilterEvent(ui->comboBoxFilter->currentIntData());
    setIsEventOnly(isEventOnly);
    emit onFilterEventChange();
}

void FilterEventPanel::on_pushButtonOverrideSearch_clicked()
{
    onPushButtonClicked(false);
    closePanel();
}

void FilterEventPanel::on_pushButtonEventOnlySearch_clicked()
{
    onPushButtonClicked(true);
    closePanel();
}

void FilterEventPanel::on_checkBoxFilter_clicked()
{
    if (ui->checkBoxFilter->isChecked()) {
        return;
    }
    setFilterEvent(INFO_MAJOR_NONE);
    setIsEventOnly(false);
    emit onFilterEventChange();
}
