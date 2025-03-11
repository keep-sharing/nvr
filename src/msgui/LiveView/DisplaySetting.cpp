#include "DisplaySetting.h"
#include "ui_DisplaySetting.h"
#include "LivePage.h"
#include "LiveView.h"
#include "LiveViewSub.h"
#include "LiveviewBottomBar.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "centralmessage.h"
DisplaySetting *DisplaySetting::s_displaySetting = nullptr;
QColor DisplaySetting::s_displayColor;

DisplaySetting::DisplaySetting(QWidget *parent)
    : BasePopup(parent)
    , ui(new Ui::DisplaySetting)
{
    ui->setupUi(this);

    s_displaySetting = this;

    //
    ui->comboBox_playMode->clear();
    ui->comboBox_playMode->addTranslatableItem("LIVEPARAMETER/41011", "Least Delay", DSP_REALTIME);
    ui->comboBox_playMode->addTranslatableItem("LIVEPARAMETER/41012", "Best Fluency", DSP_FLUENT);

    m_colorList.clear();
    ui->comboBox_color->clear();
    ui->comboBox_color->addTranslatableItem("LIVEVIEW/20200", "White", 0);
    ui->comboBox_color->setItemData(0, QColor(255, 255, 255), Qt::DecorationRole);
    m_colorList.append(QColor(255, 255, 255));
    ui->comboBox_color->addTranslatableItem("LIVEVIEW/20201", "Red", 1);
    ui->comboBox_color->setItemData(1, QColor(250, 37, 67), Qt::DecorationRole);
    m_colorList.append(QColor(250, 37, 67));
    ui->comboBox_color->addTranslatableItem("LIVEVIEW/20202", "Pink", 2);
    ui->comboBox_color->setItemData(2, QColor(206, 60, 159), Qt::DecorationRole);
    m_colorList.append(QColor(206, 60, 159));
    ui->comboBox_color->addTranslatableItem("LIVEVIEW/20203", "Purple", 3);
    ui->comboBox_color->setItemData(3, QColor(126, 41, 204), Qt::DecorationRole);
    m_colorList.append(QColor(126, 41, 204));
    ui->comboBox_color->addTranslatableItem("LIVEVIEW/20204", "Dark Blue", 4);
    ui->comboBox_color->setItemData(4, QColor(57, 76, 217), Qt::DecorationRole);
    m_colorList.append(QColor(57, 76, 217));
    ui->comboBox_color->addTranslatableItem("LIVEVIEW/20205", "Cyan", 5);
    ui->comboBox_color->setItemData(5, QColor(58, 226, 255), Qt::DecorationRole);
    m_colorList.append(QColor(58, 226, 255));
    ui->comboBox_color->addTranslatableItem("LIVEVIEW/20206", "Dark Cyan", 6);
    ui->comboBox_color->setItemData(6, QColor(48, 198, 171), Qt::DecorationRole);
    m_colorList.append(QColor(48, 198, 171));
    ui->comboBox_color->addTranslatableItem("LIVEVIEW/20207", "Dark Green", 7);
    ui->comboBox_color->setItemData(7, QColor(108, 175, 59), Qt::DecorationRole);
    m_colorList.append(QColor(108, 175, 59));
    ui->comboBox_color->addTranslatableItem("LIVEVIEW/20208", "Yellow", 8);
    ui->comboBox_color->setItemData(8, QColor(248, 210, 83), Qt::DecorationRole);
    m_colorList.append(QColor(248, 210, 83));
    connect(ui->comboBox_color, SIGNAL(activated(int)), SLOT(onDisplayInfoChanged()));

    ui->comboBox_info->clear();
    ui->comboBox_info->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBox_info->addTranslatableItem("COMMON/1012", "On", State::On);
    connect(ui->comboBox_info, SIGNAL(activated(int)), SLOT(onDisplayInfoChanged()));

    ui->comboBox_name->clear();
    ui->comboBox_name->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBox_name->addTranslatableItem("COMMON/1012", "On", State::On);
    connect(ui->comboBox_name, SIGNAL(activated(int)), SLOT(onDisplayInfoChanged()));

    ui->comboBoxChannelNameFontSize->clear();
    ui->comboBoxChannelNameFontSize->addTranslatableItem("OCCUPANCY/74229", "Small", FONT_SIZE_SMALL);
    ui->comboBoxChannelNameFontSize->addTranslatableItem("OCCUPANCY/74230", "Medium", FONT_SIZE_MEDIUM);
    ui->comboBoxChannelNameFontSize->addTranslatableItem("OCCUPANCY/74231", "Large", FONT_SIZE_LARGE);
    connect(ui->comboBoxChannelNameFontSize, SIGNAL(activated(int)), SLOT(onDisplayInfoChanged()));

    ui->comboBox_border->clear();
    ui->comboBox_border->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBox_border->addTranslatableItem("COMMON/1012", "On", State::On);
    connect(ui->comboBox_border, SIGNAL(activated(int)), SLOT(onDisplayInfoChanged()));

    ui->comboBox_page->clear();
    ui->comboBox_page->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBox_page->addTranslatableItem("COMMON/1012", "On", State::On);
    ui->comboBox_page->addTranslatableItem("COMMON/1014", "Auto", State::Auto);
    connect(ui->comboBox_page, SIGNAL(activated(int)), SLOT(onDisplayInfoChanged()));

    ui->comboBox_timeInfo->clear();
    ui->comboBox_timeInfo->addTranslatableItem("DISPLAYSETTINGS/108001", "Auto", 0);
    ui->comboBox_timeInfo->addTranslatableItem("DISPLAYSETTINGS/108002", "Always", 1);
    //ui->comboBox_timeInfo->addTranslatableItem("DISPLAYSETTINGS/108003", "Off", 2);

    ui->labelEventDetectionRegion->setTranslatableText("DISPLAYSETTINGS/108004", "Event Detection Region");
    ui->comboBoxEventDetectionRegion->clear();
    ui->comboBoxEventDetectionRegion->addTranslatableItem("DISPLAYSETTINGS/108003", "Off", 0);
    ui->comboBoxEventDetectionRegion->addTranslatableItem("SMARTEVENT/55001", "Region Entrance", EventRegionEntrance);
    ui->comboBoxEventDetectionRegion->addTranslatableItem("SMARTEVENT/55002", "Region Exiting", EventRegionExiting);
    ui->comboBoxEventDetectionRegion->addTranslatableItem("SMARTEVENT/55003", "Advanced Motion Detection", EventMotionDetection);
    ui->comboBoxEventDetectionRegion->addTranslatableItem("SMARTEVENT/55006", "Loitering", EventLoitering);
    ui->comboBoxEventDetectionRegion->addTranslatableItem("SMARTEVENT/55005", "Line Crossing", EventLineCrossing);
    ui->comboBoxEventDetectionRegion->addTranslatableItem("SMARTEVENT/55055", "Object Left/Removed", EventObject);
    ui->comboBoxEventDetectionRegion->addTranslatableItem("SMARTEVENT/55008", "People Counting", EventPeopleCounting);
    ui->comboBoxEventDetectionRegion->addTranslatableItem("REGIONAL_PEOPLECOUNTING/103313", "Regional People Counting", EventRegionalPeopleCounting);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    connect(this, SIGNAL(borderStateChanged(bool, QColor)), qMsNvr, SIGNAL(liveViewBorderChanged(bool, QColor)));

    ui->comboBox_playMode->setPermission(PERM_MODE_LIVE, PERM_LIVE_PLAYMODE);
    connect(ui->comboBox_playMode, SIGNAL(showNoPermission()), this, SLOT(showNoPermission()));
}

DisplaySetting::~DisplaySetting()
{
    s_displaySetting = nullptr;
    delete ui;
}

DisplaySetting *DisplaySetting::instance()
{
    return s_displaySetting;
}

void DisplaySetting::initializeData()
{
    struct display display_info = qMsNvr->displayInfo();

    ui->comboBox_info->setCurrentIndex(display_info.camera_info);
    ui->comboBox_name->setCurrentIndex(display_info.show_channel_name);
    ui->comboBox_border->setCurrentIndex(display_info.border_line_on);
    ui->comboBox_page->setCurrentIndex(display_info.page_info);
    ui->comboBoxEventDetectionRegion->setCurrentIndexFromData(display_info.event_region);
    ui->comboBoxChannelNameFontSize->setCurrentIndexFromData(display_info.fontSize);

    int colorIndex = get_param_int(SQLITE_FILE_NAME, PARAM_GUI_CHAN_COLOR, 0);
    ui->comboBox_color->setCurrentIndex(colorIndex);
    s_displayColor = m_colorList.at(colorIndex);

    //
    if (display_info.camera_info == 1
        || display_info.show_channel_name == 1
        || display_info.border_line_on == 1
        || (display_info.page_info == 1 || display_info.page_info == 2)) {
        emit displayStateChanged(true);
    } else {
        emit displayStateChanged(false);
    }
    //
    if (display_info.camera_info == 1
        && display_info.show_channel_name == 1
        && display_info.border_line_on == 1
        && (display_info.page_info == 1 || display_info.page_info == 2)) {
        LiveView::instance()->setDisplayMenuChecked(true);
    } else {
        LiveView::instance()->setDisplayMenuChecked(false);
    }
    //time info
    ui->comboBox_timeInfo->setCurrentIndexFromData(display_info.time_info);
    on_comboBox_timeInfo_activated(ui->comboBox_timeInfo->currentIndex());

    //
    emit borderStateChanged(display_info.border_line_on, s_displayColor);
}

void DisplaySetting::initializePlayMode()
{
    //play mode
    DSP_E playMode = static_cast<DSP_E>(get_param_int(SQLITE_FILE_NAME, PARAM_STREAM_PLAY_MODE, 0));
    qDebug() << "DisplaySetting::initializeData, REQUEST_FLAG_SET_DISPLAY_MODE, mode:" << playMode;
    ui->comboBox_playMode->setCurrentIndexFromData((int)playMode);
    sendMessageOnly(REQUEST_FLAG_SET_DISPLAY_MODE, &playMode, sizeof(playMode));
}

void DisplaySetting::setPos(const QPoint &p)
{
    m_pos = p;
}

QPoint DisplaySetting::calculatePos()
{
    return m_pos;
}

void DisplaySetting::closePopup(BasePopup::CloseType type)
{
    Q_UNUSED(type)
}

void DisplaySetting::saveData()
{
    struct display display_info = qMsNvr->displayInfo();

    display_info.camera_info = ui->comboBox_info->currentIndex();
    display_info.show_channel_name = ui->comboBox_name->currentIndex();
    display_info.border_line_on = ui->comboBox_border->currentIndex();
    display_info.page_info = ui->comboBox_page->currentIndex();
    display_info.time_info = ui->comboBox_timeInfo->currentData().toInt();
    display_info.event_region = ui->comboBoxEventDetectionRegion->currentData().toInt();
    display_info.fontSize = static_cast<FontSize>(ui->comboBoxChannelNameFontSize->currentIntData());

    qMsNvr->writeDisplayInfo(&display_info);

    int colorIndex = ui->comboBox_color->currentIndex();
    s_displayColor = m_colorList.at(colorIndex);
    qMsNvr->writeDisplayColor(colorIndex, s_displayColor);

    //page
    if (LivePage::mainPage()) {
        switch (display_info.page_info) {
        case State::Off:
            LivePage::mainPage()->setMode(LivePage::ModeAlwaysHide);
            break;
        case State::On:
            LivePage::mainPage()->setMode(LivePage::ModeAlwaysShow);
            break;
        case State::Auto:
            LivePage::mainPage()->setMode(LivePage::ModeAuto);
            break;
        }
    }

    //
    if (display_info.camera_info == 1
        || display_info.show_channel_name == 1
        || display_info.border_line_on == 1
        || (display_info.page_info == 1 || display_info.page_info == 2)) {
        emit displayStateChanged(true);
    } else {
        emit displayStateChanged(false);
    }
    //
    if (display_info.camera_info == 1
        && display_info.show_channel_name == 1
        && display_info.border_line_on == 1
        && (display_info.page_info == 1 || display_info.page_info == 2)) {
        LiveView::instance()->setDisplayMenuChecked(true);
    } else {
        LiveView::instance()->setDisplayMenuChecked(false);
    }

    //
    emit borderStateChanged(display_info.border_line_on, s_displayColor);
}

void DisplaySetting::mousePressEvent(QMouseEvent *event)
{
    BasePopup::mousePressEvent(event);
}

void DisplaySetting::showEvent(QShowEvent *event)
{
    initializeData();

    BasePopup::showEvent(event);
}

void DisplaySetting::hideEvent(QHideEvent *event)
{
    BasePopup::hideEvent(event);
}

void DisplaySetting::onLanguageChanged()
{
    ui->label_playMode->setText(GET_TEXT("LIVEPARAMETER/41010", "Play Mode"));
    ui->comboBox_playMode->retranslate();
    ui->label_color->setText(GET_TEXT("LIVEVIEW/20073", "Color"));
    ui->comboBox_color->retranslate();
    ui->label_streamInfo->setText(GET_TEXT("LIVEVIEW/20034", "Stream Info"));
    ui->comboBox_info->retranslate();
    ui->label_channelName->setText(GET_TEXT("CHANNELMANAGE/30009", "Channel Name"));
    ui->comboBox_name->retranslate();
    ui->label_borderLine->setText(GET_TEXT("LIVEPARAMETER/41000", "Borderline"));
    ui->comboBox_border->retranslate();
    ui->label_pageInfo->setText(GET_TEXT("LIVEPARAMETER/41009", "Page Info"));
    ui->comboBox_page->retranslate();
    ui->label_timeInfo->setText(GET_TEXT("DISPLAYSETTINGS/108000", "Time Info"));
    ui->comboBox_timeInfo->retranslate();
    ui->labelEventDetectionRegion->retranslate();
    ui->comboBoxEventDetectionRegion->retranslate();

    ui->labelChannelNameFontSize->setText(GET_TEXT("LIVEVIEW/155000", "Channel Name Font Size"));
    ui->comboBoxChannelNameFontSize->retranslate();
}

void DisplaySetting::onDisplayInfoChanged()
{
    saveData();

    LiveView::instance()->changeDisplayInfo();
}

void DisplaySetting::on_comboBox_playMode_activated(int index)
{
    DSP_E playMode = static_cast<DSP_E>(ui->comboBox_playMode->itemData(index).toInt());
    qDebug() << "REQUEST_FLAG_SET_DISPLAY_MODE, mode:" << playMode;
    set_param_int(SQLITE_FILE_NAME, PARAM_STREAM_PLAY_MODE, (int)playMode);
    sendMessageOnly(REQUEST_FLAG_SET_DISPLAY_MODE, &playMode, sizeof(playMode));
}

void DisplaySetting::on_comboBox_timeInfo_activated(int index)
{
    saveData();

    int mode = ui->comboBox_timeInfo->itemData(index).toInt();
    emit timeInfoModeChanged(mode);
    //
    LiveView::instance()->setTimeInfoMode(mode);
    if (LiveViewSub::instance()) {
        LiveViewSub::instance()->setTimeInfoMode(mode);
    }
}

void DisplaySetting::on_comboBoxEventDetectionRegion_activated(int index)
{
    struct display display_info = qMsNvr->displayInfo();
    int event_region = ui->comboBoxEventDetectionRegion->itemData(index).toInt();
    if (display_info.event_region != event_region) {
        display_info.event_region = event_region;
        qMsNvr->writeDisplayInfo(&display_info);
    }
}

void DisplaySetting::showNoPermission()
{
    ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
}
