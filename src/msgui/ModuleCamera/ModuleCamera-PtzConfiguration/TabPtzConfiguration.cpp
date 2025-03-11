#include "TabPtzConfiguration.h"
#include "ui_TabPtzConfiguration.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "addpatrolkeypoint.h"
#include "centralmessage.h"
#include "msuser.h"
#include "presettableitemdelegate.h"
#include <QtDebug>

extern "C" {
#include "ptz_public.h"
}
//此页面已弃用
TabPtzConfiguration::TabPtzConfiguration(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::PtzConfigurationPage)
{
    ui->setupUi(this);

    //channel
    ui->comboBox_channel->clear();
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        ui->comboBox_channel->addItem(QString::number(i + 1), i);
    }

    ui->ptzControlPanel->setPresetVisible(false);

    m_itemDelegate = new PresetTableItemDelegate(this);

    initializePreset();
    initializePatrol();
    initializePattern();

    onLanguageChanged();
}

TabPtzConfiguration::~TabPtzConfiguration()
{
    delete ui;
}

void TabPtzConfiguration::initializeData()
{
    m_itemDelegate->setEditable(true);
    //
    adjustTableItemWidth();
    //
    ui->comboBox_channel->setCurrentIndex(currentChannel());
    on_comboBox_channel_activated(ui->comboBox_channel->currentIndex());
}

void TabPtzConfiguration::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabPtzConfiguration::adjustTableItemWidth()
{
    int w1 = (ui->widget_right->width() - 60) / (ui->tableView_preset->columnCount() - 1);
    for (int column = 0; column < ui->tableView_preset->columnCount(); ++column) {
        ui->tableView_preset->setColumnWidth(column, w1);
    }
    int w2 = (ui->widget_right->width() - 60) / (ui->tableView_patrol->columnCount() - 1);
    for (int column = 0; column < ui->tableView_patrol->columnCount(); ++column) {
        ui->tableView_patrol->setColumnWidth(column, w2);
    }
}

void TabPtzConfiguration::setPtzEnable(bool enable)
{
    ui->ptzControlPanel->setEnabled(enable);
    ui->groupBox_preset->setEnabled(enable);
    ui->groupBox_patrol->setEnabled(enable);
    ui->groupBox_pattern->setEnabled(enable);

    //
    if (enable) {
        //某些不支持preset的机型，PTZ的控制面板中preset ，path，pattern图标需置灰
        if (!gPtzDataManager->isPresetEnable()) {
            ui->groupBox_preset->setEnabled(false);
            ui->groupBox_patrol->setEnabled(false);
            ui->groupBox_pattern->setEnabled(false);
            //
            int zoom = gPtzDataManager->waitForGetPtzZoomPos();
            ui->ptzControlPanel->setZoomValue(zoom);
        }
    } else {
        ui->tableView_preset->clearContent();
        ui->tableView_patrol->clearContent();
    }
}

void TabPtzConfiguration::initializePreset()
{
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("PTZCONFIG/36021", "Preset No.");
    headerList << GET_TEXT("PTZCONFIG/36022", "Play");
    headerList << GET_TEXT("COMMON/1036", "Save");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView_preset->setHorizontalHeaderLabels(headerList);
    ui->tableView_preset->setColumnCount(headerList.size());
    ui->tableView_preset->setSortingEnabled(false);
    ui->tableView_preset->hideColumn(PresetColumnCheck);
    //delegate
    ui->tableView_preset->setItemDelegate(m_itemDelegate);
    ui->tableView_preset->setEditTriggers(QTableView::DoubleClicked | QTableView::EditKeyPressed | QTableView::AnyKeyPressed);
    //
    connect(ui->tableView_preset, SIGNAL(itemClicked(int, int)), this, SLOT(onPresetItemClicked(int, int)));
}

void TabPtzConfiguration::showPresetData(const resp_ptz_preset *preset_array, int count)
{
    ui->tableView_preset->clearContent();

    for (int i = 0; i < count; ++i) {
        //
        const resp_ptz_preset &preset = preset_array[i];
        if (QString(preset.name).isEmpty()) {
            ui->tableView_preset->setItemText(i, PresetColumnNumber, QString("Preset %1").arg(i + 1));
        } else {
            ui->tableView_preset->setItemText(i, PresetColumnNumber, preset.name);
        }

        //固定值
        if (i >= 32 && i <= 51) {
            ui->tableView_preset->setRowData(i, PresetTableItemDelegate::ItemValidDefault, PresetTableItemDelegate::ItemStateRole);
        } else {
            if (preset.enable) {
                ui->tableView_preset->setRowData(i, PresetTableItemDelegate::ItemNormal, PresetTableItemDelegate::ItemStateRole);
            } else {
                ui->tableView_preset->setRowData(i, PresetTableItemDelegate::ItemDisable, PresetTableItemDelegate::ItemStateRole);
            }
        }

        //MS-Cxx61机型的PTZ控制面板中，预置点中auto flip；tilt scan；panorama scan应置灰(无效)
        switch (i) {
        case 32:
        case 50:
        case 51:
            if (gPtzDataManager->isPtzBullet()) {
                ui->tableView_preset->setRowData(i, PresetTableItemDelegate::ItemInvalidDefault, PresetTableItemDelegate::ItemStateRole);
            }
            break;
        }
    }
}

void TabPtzConfiguration::initializePatrol()
{
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("PTZCONFIG/36023", "Key Point");
    headerList << GET_TEXT("PTZDIALOG/21015", "Preset");
    headerList << GET_TEXT("PTZCONFIG/36026", "Scan Speed");
    headerList << GET_TEXT("PTZCONFIG/36025", "Scan Time");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView_patrol->setHorizontalHeaderLabels(headerList);
    ui->tableView_patrol->setColumnCount(headerList.size());
    ui->tableView_patrol->setSortingEnabled(false);
    ui->tableView_patrol->hideColumn(PresetColumnCheck);
    //delegate
    ui->tableView_patrol->setItemDelegateForColumn(PatrolColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView_patrol->setItemDelegateForColumn(PatrolColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    connect(ui->tableView_patrol, SIGNAL(itemClicked(int, int)), this, SLOT(onPatrolItemClicked(int, int)));
}

void TabPtzConfiguration::showPatrolData(const resp_ptz_tour *tour_array, int count)
{
    ui->comboBox_patrol->clear();
    m_patrolMap.clear();

    for (int i = 0; i < count; ++i) {
        const resp_ptz_tour &tour = tour_array[i];
        ui->comboBox_patrol->addItem(QString("Path %1").arg(i + 1));
        m_patrolMap.insert(i, tour);
    }

    on_comboBox_patrol_activated(0);
}

void TabPtzConfiguration::setPatrolData(int tourid)
{
    QList<ptz_path> pathList;
    for (int row = 0; row < ui->tableView_patrol->rowCount(); ++row) {
        int preset = ui->tableView_patrol->itemText(row, PatrolColumnPreset).toInt();
        int time = ui->tableView_patrol->itemText(row, PatrolColumnTime).toInt();
        int speed = ui->tableView_patrol->itemText(row, PatrolColumnSpeed).toInt();

        ptz_path path;
        path.preset_id = preset;
        path.timeout = time;
        path.speed = speed;
        pathList.append(path);
    }
    gPtzDataManager->sendPatrolData(tourid, pathList);
}

void TabPtzConfiguration::updatePatrolDataFromTable()
{
    resp_ptz_tour &tour = m_patrolMap[ui->comboBox_patrol->currentIndex()];
    memset(&tour, 0, sizeof(resp_ptz_tour));
    for (int row = 0; row < ui->tableView_patrol->rowCount(); ++row) {
        int preset = ui->tableView_patrol->itemText(row, PatrolColumnPreset).toInt();
        int time = ui->tableView_patrol->itemText(row, PatrolColumnTime).toInt();
        int speed = ui->tableView_patrol->itemText(row, PatrolColumnSpeed).toInt();
        tour.preset_id[row] = preset;
        tour.timeout[row] = time;
        tour.speed[row] = speed;
    }
}

void TabPtzConfiguration::initializePattern()
{
}

void TabPtzConfiguration::showPatternData(const int *pattern_array, int count)
{
    ui->comboBox_pattern->clear();
    //
    for (int i = 0; i < count; ++i) {
        const int &enable = pattern_array[i];
        ui->comboBox_pattern->addItem(QString("Pattern %1").arg(i + 1), enable);
    }
}

void TabPtzConfiguration::onLanguageChanged()
{
    ui->ptzControlPanel->onLanguageChanged();

    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->groupBox_preset->setTitle(GET_TEXT("PTZDIALOG/21015", "Preset"));
    ui->groupBox_patrol->setTitle(GET_TEXT("PTZDIALOG/21016", "Patrol"));
    ui->groupBox_pattern->setTitle(GET_TEXT("PTZDIALOG/21017", "Pattern"));

    ui->toolButton_patrol_add->setToolTip(GET_TEXT("COMMON/1000", "Add"));
    ui->toolButton_patrol_up->setToolTip(GET_TEXT("PTZCONFIG/36034", "Up"));
    ui->toolButton_patrol_down->setToolTip(GET_TEXT("PTZCONFIG/36035", "Down"));
    ui->toolButton_patrol_play->setToolTip(GET_TEXT("PTZCONFIG/36037", "Run"));
    ui->toolButton_patrol_stop->setToolTip(GET_TEXT("PTZCONFIG/36038", "Stop"));
    ui->toolButton_patrol_delete->setToolTip(GET_TEXT("PTZCONFIG/36039", "Clear"));
    if (ui->toolButton_pattern_record->isChecked()) {
        ui->toolButton_pattern_record->setToolTip(GET_TEXT("COMMON/1036", "Save"));
    } else {
        ui->toolButton_pattern_record->setToolTip(GET_TEXT("MENU/10004", "Record"));
    }
    ui->toolButton_pattern_play->setToolTip(GET_TEXT("PTZCONFIG/36037", "Run"));
    ui->toolButton_pattern_stop->setToolTip(GET_TEXT("PTZCONFIG/36038", "Stop"));
    ui->toolButton_pattern_delete->setToolTip(GET_TEXT("PTZCONFIG/36039", "Clear"));

    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabPtzConfiguration::on_comboBox_channel_activated(int index)
{
    m_channel = ui->comboBox_channel->itemData(index).toInt();
    setCurrentChannel(m_channel);
    ui->widget_video->playVideo(m_channel);
    if (!LiveView::instance()->isChannelConnected(m_channel)) {
        setPtzEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    } else {
        ui->widgetMessage->hide();
    }

    //showWait();
    gPtzDataManager->beginGetData(m_channel);
    gPtzDataManager->waitForGetCameraModelType();
    if (qMsNvr->isFisheye(m_channel)) {
        setPtzEnable(false);
        //closeWait();
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    } else {
        ui->widgetMessage->hide();
    }
    gPtzDataManager->waitForGetCameraModelInfo();
    gPtzDataManager->waitForGetPtzSupport();
    int support;
    bool isMsCamera = qMsNvr->isMsCamera(m_channel);
    if (isMsCamera) {
        support = gPtzDataManager->ptzSupport();
    } else {
        support = PTZ_SUPPORT_NONE;
    }
    if (!(support & PTZ_SUPPORT_YES)) {
        setPtzEnable(false);
        //closeWait();
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
    } else {
        ui->widgetMessage->hide();
        setPtzEnable(true);
        //MSHN-6191 QT-Live View：IPC型号中带“F”的，PTZ的控制面板中zoom为拉条形式
        ui->ptzControlPanel->setAutoZoomModel(support & PTZ_SUPPORT_ZOOM_SLIDER);
        //枪机zoom置灰
        ui->ptzControlPanel->setZoomEnable(support & PTZ_SUPPORT_ZOOM_SLIDER || support & PTZ_SUPPORT_ZOOM_KEY);
        //【【旧】PTZ：测试设备为普通枪机（MS-C8152-PA），Lens Type选择P-Iris，Iris+/-置灰无法操作】https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001104365
        ui->ptzControlPanel->setFocusEnable(support & PTZ_SUPPORT_FOCUS);
        gPtzDataManager->waitForGetPtzOvfInfo();
        //
        resp_ptz_ovf_info *ptz_ovf_info = gPtzDataManager->ptzOvfInfo();
        if (gPtzDataManager->isPresetEnable()) {
            showPresetData(ptz_ovf_info->preset, gPtzDataManager->maxPresetCount(m_channel));
            showPatrolData(ptz_ovf_info->tour, TOUR_MAX);
            showPatternData(ptz_ovf_info->pattern, PATTERN_MAX);
        } else {
            ui->tableView_preset->clearContent();
            ui->tableView_patrol->clearContent();
            ui->comboBox_pattern->clear();
        }

        gPtzDataManager->waitForGetPtzSpeed();
        ui->ptzControlPanel->editSpeedValue(gPtzDataManager->ptzSpeed());

        gPtzDataManager->waitForGetAutoScan();
        ui->ptzControlPanel->setAutoScanChecked(gPtzDataManager->autoScan());

        //closeWait();
    }
}

void TabPtzConfiguration::onPresetItemClicked(int row, int column)
{
    const PresetTableItemDelegate::ItemType &state = static_cast<PresetTableItemDelegate::ItemType>(ui->tableView_preset->itemData(row, column, PresetTableItemDelegate::ItemStateRole).toInt());
    const QString &name = ui->tableView_preset->itemText(row, PresetColumnNumber);
    switch (column) {
    case PresetColumnPlay: {
        switch (state) {
        case PresetTableItemDelegate::ItemNormal:
        case PresetTableItemDelegate::ItemValidDefault:
            gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, row + 1, ui->ptzControlPanel->speedValue(), name);
            break;
        default:
            break;
        }
        break;
    }
    case PresetColumnSave: {
        switch (state) {
        case PresetTableItemDelegate::ItemNormal:
        case PresetTableItemDelegate::ItemDisable:
            gPtzDataManager->sendPresetControl(PTZ_PRESET_SET, row + 1, ui->ptzControlPanel->speedValue(), name);
            ui->tableView_preset->setRowData(row, PresetTableItemDelegate::ItemNormal, PresetTableItemDelegate::ItemStateRole);
            break;
        default:
            break;
        }
        break;
    }
    case PresetColumnDelete: {
        switch (state) {
        case PresetTableItemDelegate::ItemNormal:
            gPtzDataManager->sendPresetControl(PTZ_PRESET_CLEAR, row + 1, ui->ptzControlPanel->speedValue(), name);
            ui->tableView_preset->setRowData(row, PresetTableItemDelegate::ItemDisable, PresetTableItemDelegate::ItemStateRole);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void TabPtzConfiguration::on_comboBox_patrol_activated(int index)
{
    ui->tableView_patrol->clearContent();

    const resp_ptz_tour &tour = m_patrolMap.value(index);
    for (int i = 0; i < MS_KEY_MAX; ++i) {
        int preset = tour.preset_id[i];
        if (preset == 0) {
            break;
        }
        int time = tour.timeout[i];
        int speed = tour.speed[i];
        ui->tableView_patrol->setItemText(i, PatrolColumnPoint, QString::number(i + 1));
        ui->tableView_patrol->setItemText(i, PatrolColumnPreset, QString::number(preset));
        ui->tableView_patrol->setItemText(i, PatrolColumnTime, QString::number(time));
        ui->tableView_patrol->setItemText(i, PatrolColumnSpeed, QString::number(speed));
    }
}

void TabPtzConfiguration::onPatrolItemClicked(int row, int column)
{
    switch (column) {
    case PatrolColumnEdit: {
        AddPatrolKeyPoint editPoint(AddPatrolKeyPoint::ModeEdit, m_channel, this);
        if (gPtzDataManager->isPtzBullet()) {
            editPoint.setScanTimeRange(15, 120);
        } else {
            editPoint.setScanTimeRange(10, 120);
        }
        editPoint.showKeyPointEdit(row);
        editPoint.setPresetPoint(ui->tableView_patrol->itemText(row, PatrolColumnPreset).toInt());
        editPoint.setScanSpeed(ui->tableView_patrol->itemText(row, PatrolColumnSpeed).toInt());
        editPoint.setScanTime(ui->tableView_patrol->itemText(row, PatrolColumnTime).toInt());
        const int &result = editPoint.exec();
        if (result == AddPatrolKeyPoint::Accepted) {
            int preset = editPoint.presetPoint();
            int time = editPoint.scanTime();
            int speed = editPoint.scanSpeed();
            ui->tableView_patrol->setItemText(row, PatrolColumnPreset, QString::number(preset));
            ui->tableView_patrol->setItemText(row, PatrolColumnTime, QString::number(time));
            ui->tableView_patrol->setItemText(row, PatrolColumnSpeed, QString::number(speed));
            //
            int tourid = ui->comboBox_patrol->currentIndex() + 1;
            setPatrolData(tourid);
            updatePatrolDataFromTable();
        }
        break;
    }
    case PatrolColumnDelete: {
        ui->tableView_patrol->removeRow(row);
        int tourid = ui->comboBox_patrol->currentIndex() + 1;
        setPatrolData(tourid);
        updatePatrolDataFromTable();
        on_comboBox_patrol_activated(tourid - 1);
        break;
    }
    default:
        break;
    }
}

void TabPtzConfiguration::on_toolButton_patrol_add_clicked()
{
    ui->toolButton_patrol_add->clearUnderMouse();

    int index = ui->tableView_patrol->rowCount();
    if (index >= MS_KEY_MAX) {
        return;
    }

    AddPatrolKeyPoint addPoint(AddPatrolKeyPoint::ModeAdd, m_channel, this);
    if (gPtzDataManager->isPtzBullet()) {
        addPoint.setScanTimeRange(15, 120);
    } else {
        addPoint.setScanTimeRange(10, 120);
    }
    addPoint.showKeyPointAdd(index);
    const int &result = addPoint.exec();
    if (result == AddPatrolKeyPoint::Accepted) {
        int preset = addPoint.presetPoint();
        int time = addPoint.scanTime();
        int speed = addPoint.scanSpeed();
        ui->tableView_patrol->setItemText(index, PatrolColumnPoint, QString::number(index + 1));
        ui->tableView_patrol->setItemText(index, PatrolColumnPreset, QString::number(preset));
        ui->tableView_patrol->setItemText(index, PatrolColumnTime, QString::number(time));
        ui->tableView_patrol->setItemText(index, PatrolColumnSpeed, QString::number(speed));
        //
        int tourid = ui->comboBox_patrol->currentIndex() + 1;
        setPatrolData(tourid);
        updatePatrolDataFromTable();
    }
}

void TabPtzConfiguration::on_toolButton_patrol_up_clicked()
{
    ui->toolButton_patrol_up->clearUnderMouse();

    int currentRow = ui->tableView_patrol->currentRow();
    if (currentRow < 0) {
        return;
    }
    int destRow = currentRow - 1;
    if (destRow < 0) {
        return;
    }
    QString temp1 = ui->tableView_patrol->itemText(destRow, PatrolColumnPreset);
    QString temp2 = ui->tableView_patrol->itemText(destRow, PatrolColumnTime);
    QString temp3 = ui->tableView_patrol->itemText(destRow, PatrolColumnSpeed);
    ui->tableView_patrol->setItemText(destRow, PatrolColumnPreset, ui->tableView_patrol->itemText(currentRow, PatrolColumnPreset));
    ui->tableView_patrol->setItemText(destRow, PatrolColumnTime, ui->tableView_patrol->itemText(currentRow, PatrolColumnTime));
    ui->tableView_patrol->setItemText(destRow, PatrolColumnSpeed, ui->tableView_patrol->itemText(currentRow, PatrolColumnSpeed));
    ui->tableView_patrol->setItemText(currentRow, PatrolColumnPreset, temp1);
    ui->tableView_patrol->setItemText(currentRow, PatrolColumnTime, temp2);
    ui->tableView_patrol->setItemText(currentRow, PatrolColumnSpeed, temp3);
    ui->tableView_patrol->selectRow(destRow);
    //
    int tourid = currentRow + 1;
    setPatrolData(tourid);
    updatePatrolDataFromTable();
}

void TabPtzConfiguration::on_toolButton_patrol_down_clicked()
{
    ui->toolButton_patrol_down->clearUnderMouse();

    int currentRow = ui->tableView_patrol->currentRow();
    if (currentRow < 0) {
        return;
    }
    int destRow = currentRow + 1;
    if (destRow >= ui->tableView_patrol->rowCount()) {
        return;
    }
    QString temp1 = ui->tableView_patrol->itemText(destRow, PatrolColumnPreset);
    QString temp2 = ui->tableView_patrol->itemText(destRow, PatrolColumnTime);
    QString temp3 = ui->tableView_patrol->itemText(destRow, PatrolColumnSpeed);
    ui->tableView_patrol->setItemText(destRow, PatrolColumnPreset, ui->tableView_patrol->itemText(currentRow, PatrolColumnPreset));
    ui->tableView_patrol->setItemText(destRow, PatrolColumnTime, ui->tableView_patrol->itemText(currentRow, PatrolColumnTime));
    ui->tableView_patrol->setItemText(destRow, PatrolColumnSpeed, ui->tableView_patrol->itemText(currentRow, PatrolColumnSpeed));
    ui->tableView_patrol->setItemText(currentRow, PatrolColumnPreset, temp1);
    ui->tableView_patrol->setItemText(currentRow, PatrolColumnTime, temp2);
    ui->tableView_patrol->setItemText(currentRow, PatrolColumnSpeed, temp3);
    ui->tableView_patrol->selectRow(destRow);
    //
    int tourid = currentRow + 1;
    setPatrolData(tourid);
    updatePatrolDataFromTable();
}

void TabPtzConfiguration::on_toolButton_patrol_play_clicked()
{
    ui->toolButton_patrol_play->clearUnderMouse();

    int tourid = ui->comboBox_patrol->currentIndex() + 1;
    gPtzDataManager->sendPatrolControl(REQUEST_FLAG_PTZ_TOUR_RUN, tourid);
}

void TabPtzConfiguration::on_toolButton_patrol_stop_clicked()
{
    ui->toolButton_patrol_stop->clearUnderMouse();

    int tourid = ui->comboBox_patrol->currentIndex() + 1;
    gPtzDataManager->sendPatrolControl(REQUEST_FLAG_PTZ_TOUR_STOP, tourid);
}

void TabPtzConfiguration::on_toolButton_patrol_delete_clicked()
{
    ui->toolButton_patrol_delete->clearUnderMouse();

    const int &result = MessageBox::question(this, GET_TEXT("PTZCONFIG/36008", "Are you sure to clear Path%1?").arg(ui->comboBox_patrol->currentIndex() + 1));
    if (result == MessageBox::Yes) {
        int tourid = ui->comboBox_patrol->currentIndex() + 1;
        ui->tableView_patrol->clearContent();
        updatePatrolDataFromTable();
        gPtzDataManager->sendPatrolControl(REQUEST_FLAG_PTZ_TOUR_CLEAR, tourid);
    }
}

void TabPtzConfiguration::on_toolButton_pattern_record_clicked(bool checked)
{
    ui->toolButton_pattern_record->clearUnderMouse();

    int patternid = ui->comboBox_pattern->currentIndex() + 1;
    int speed = ui->ptzControlPanel->speedValue();
    if (checked) {
        gPtzDataManager->sendPatternControl(PtzDataManager::PatternStartRecord, patternid, speed);
        ui->toolButton_pattern_record->setToolTip(GET_TEXT("COMMON/1036", "Save"));
    } else {
        gPtzDataManager->sendPatternControl(PtzDataManager::PatternStopRecord, patternid, speed);
        ui->toolButton_pattern_record->setToolTip(GET_TEXT("MENU/10004", "Record"));
    }
}

void TabPtzConfiguration::on_toolButton_pattern_play_clicked()
{
    ui->toolButton_pattern_play->clearUnderMouse();

    int patternid = ui->comboBox_pattern->currentIndex() + 1;
    int speed = ui->ptzControlPanel->speedValue();
    gPtzDataManager->sendPatternControl(PtzDataManager::PatternRun, patternid, speed);
}

void TabPtzConfiguration::on_toolButton_pattern_stop_clicked()
{
    ui->toolButton_pattern_stop->clearUnderMouse();

    int patternid = ui->comboBox_pattern->currentIndex() + 1;
    int speed = ui->ptzControlPanel->speedValue();
    gPtzDataManager->sendPatternControl(PtzDataManager::PatternStop, patternid, speed);
}

void TabPtzConfiguration::on_toolButton_pattern_delete_clicked()
{
    ui->toolButton_pattern_delete->clearUnderMouse();

    const int &result = MessageBox::question(this, GET_TEXT("PTZCONFIG/36006", "Do you want to clear Pattern%1?").arg(ui->comboBox_pattern->currentIndex() + 1));
    if (result == MessageBox::Yes) {
        int patternid = ui->comboBox_pattern->currentIndex() + 1;
        int speed = ui->ptzControlPanel->speedValue();
        gPtzDataManager->sendPatternControl(PtzDataManager::PatternDelete, patternid, speed);
    }
}

void TabPtzConfiguration::on_pushButton_back_clicked()
{
    back();
}
