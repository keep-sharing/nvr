#include "manualmodesettings.h"
#include "ui_manualmodesettings.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "centralmessage.h"
#include "editmanualmode.h"
#include <QtDebug>

int ExposureTimeIndexRole = Qt::UserRole + 90;
int IsIpcTypeRole = Qt::UserRole + 100;
int IsNvrTypeRole = Qt::UserRole + 101;

ManualModeSettings::ManualModeSettings(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::ManualModeSettings)
{
    ui->setupUi(this);
    setTitleWidget(ui->label_title);

    //initialize table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("IMAGE/37213", "Manual Mode No.");
    headerList << GET_TEXT("IMAGE/37210", "Exposure Time");
    headerList << GET_TEXT("IMAGE/37211", "Gain Level");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->hideColumn(ColumnCheck);
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(itemDoubleClicked(int, int)), this, SLOT(onTableItemDoubleClicked(int, int)));
    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //sort
    ui->tableView->setSortingEnabled(false);

    m_waitting = new MsWaitting(this);

    onLanguageChanged();
}

ManualModeSettings::~ManualModeSettings()
{
    clearCache();
    delete ui;
}

void ManualModeSettings::initializeData(int channel)
{
    m_channel = channel;
    if (m_exposureShce) {
        memset(m_exposureShceCache, 0, sizeof(struct exposure_schedule));
        memcpy(m_exposureShceCache, m_exposureShce, sizeof(struct exposure_schedule));
        m_exposureListCache.clear();
        for (auto i = m_exposureList.begin(); i != m_exposureList.end(); ++i) {
            m_exposureListCache.append(exposureType(i->exposure.exposureTime, i->exposure.gainLevel));
        }
    }
    updateTable();
}

bool ManualModeSettings::isModeExist(int exposure_time, int gain_level, int edit_index) const
{
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        if (i == edit_index) {
            continue;
        }
        int exposureTime = ui->tableView->itemData(i, ColumnExposureTime, ExposureTimeIndexRole).toInt();
        int gainLevel = ui->tableView->itemIntValue(i, ColumnGainLevel);
        if (exposureTime == exposure_time && gainLevel == gain_level) {
            return true;
        }
    }
    return false;
}

void ManualModeSettings::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }

    switch (message->type()) {
    case RESPONSE_FLAG_GET_EXPOSURE_SCHE:
        ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE(message);
        message->accept();
        break;
    default:
        break;
    }
}

void ManualModeSettings::resizeEvent(QResizeEvent *)
{
    int columnWidth = (width() - 20) / (ui->tableView->columnCount() - 1);
    for (int i = ColumnModeIndex; i < ColumnDelete; ++i) {
        ui->tableView->setColumnWidth(i, columnWidth);
    }
}

void ManualModeSettings::closeEvent(QCloseEvent *)
{
}

void ManualModeSettings::ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE(MessageReceive *message)
{
    //m_waitting->//closeWait();
    struct exposure_schedule *data = (struct exposure_schedule *)message->data;
    if (!data) {
        qWarning() << "ManualModeSettings::ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE, data is null.";
        return;
    }
    if (!m_exposureShce) {
        m_exposureShce = new exposure_schedule;
    }
    if (!m_exposureShceCache) {
        m_exposureShceCache = new exposure_schedule;
    }
    memcpy(m_exposureShce, data, sizeof(struct exposure_schedule));
    memcpy(m_exposureShceCache, data, sizeof(struct exposure_schedule));
    //初始化List
    for (int day = 0; day < MAX_DAY_NUM; ++day) {
        const exposure_schedule_day &schedule_day = m_exposureShce->schedule_day[day];
        for (int i = 0; i < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; ++i) {
            const exposure_schedule_item &schedule_item = schedule_day.schedule_item[i];
            ExposureManualValue manualValue(schedule_item.action_type, schedule_item.exposureTime, schedule_item.gainLevel);
            exposureType exposureValue(schedule_item.exposureTime, schedule_item.gainLevel);
            if (manualValue.isValid() && !m_exposureList.contains(exposureValue)) {
                m_exposureList.append(exposureValue);
            }
        }
    }
}

void ManualModeSettings::updateTable()
{
    ui->tableView->clearContent();
    int row = 0;

    QMap<ExposureManualValue, int> mapManualMode;
    //先遍历出来ipc返回的ExposureManualValue
    for (int day = 0; day < MAX_DAY_NUM; ++day) {
        const exposure_schedule_day &schedule_day = m_exposureShce->schedule_day[day];
        for (int i = 0; i < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; ++i) {
            const exposure_schedule_item &schedule_item = schedule_day.schedule_item[i];
            ExposureManualValue manualValue(schedule_item.action_type, schedule_item.exposureTime, schedule_item.gainLevel);
            if (manualValue.isValid() && !mapManualMode.contains(manualValue)) {
                ui->tableView->setItemIntValue(row, ColumnModeIndex, row + 1);
                ui->tableView->setItemData(row, ColumnModeIndex, true, IsIpcTypeRole);
                ui->tableView->setItemText(row, ColumnExposureTime, exposureTimeString(schedule_item.exposureTime));
                ui->tableView->setItemData(row, ColumnExposureTime, schedule_item.exposureTime, ExposureTimeIndexRole);
                ui->tableView->setItemIntValue(row, ColumnGainLevel, schedule_item.gainLevel);
                mapManualMode.insert(manualValue, 0);
                row++;
            }
        }
    }
    qDebug() << QString("ManualModeSettings::updateTable, exposure_schedule, mode size: %1").arg(mapManualMode.size());

    //再读取nvr的
    qDebug() << QString("ManualModeSettings::updateTable, read_exposure, mode size: %1").arg(m_exposureList.count());
    for (auto i = m_exposureList.begin(); i != m_exposureList.end(); ++i) {
        const struct exposure &e = i->exposure;
        ExposureManualValue manualValue(ExposureActionManual, e.exposureTime, e.gainLevel);
        if (manualValue.isValid() && !mapManualMode.contains(manualValue)) {
            ui->tableView->setItemIntValue(row, ColumnModeIndex, row + 1);
            ui->tableView->setItemData(row, ColumnModeIndex, true, IsNvrTypeRole);
            ui->tableView->setItemText(row, ColumnExposureTime, exposureTimeString(e.exposureTime));
            ui->tableView->setItemData(row, ColumnExposureTime, e.exposureTime, ExposureTimeIndexRole);
            ui->tableView->setItemIntValue(row, ColumnGainLevel, e.gainLevel);
            mapManualMode.insert(manualValue, 0);
            row++;
        }
    }

    ui->pushButton_add->setEnabled(ui->tableView->rowCount() < 70);
    //
}

void ManualModeSettings::setExposureCtrl(int type)
{
    m_exposureCtrl = type;
}

void ManualModeSettings::clearCache()
{
    if (m_exposureShce) {
        delete m_exposureShce;
        m_exposureShce = nullptr;
    }
    if (m_exposureShceCache) {
        delete m_exposureShceCache;
        m_exposureShceCache = nullptr;
    }
    m_exposureList.clear();
    m_exposureListCache.clear();
}

void ManualModeSettings::apply()
{
    saveData();
    clearCache();
}

QString ManualModeSettings::exposureTimeString(int index) const
{
    QString text;
    if (!m_exposureCtrl) {
        switch (index) {
        case 0:
            text = "1/5";
            break;
        case 1:
            text = "1/15";
            break;
        case 2:
            text = "1/30";
            break;
        case 3:
            text = "1/60";
            break;
        case 4:
            text = "1/120";
            break;
        case 5:
            text = "1/250";
            break;
        case 6:
            text = "1/500";
            break;
        case 7:
            text = "1/750";
            break;
        case 8:
            text = "1/1000";
            break;
        case 9:
            text = "1/2000";
            break;
        case 10:
            text = "1/4000";
            break;
        case 11:
            text = "1/10000";
            break;
        case 12:
            text = "1/100000";
            break;
        case 13:
            text = "1";
            break;
        }
    } else {
        switch (index) {
        case 0:
            text = "1/5";
            break;
        case 1:
            text = "1/10";
            break;
        case 2:
            text = "1/25";
            break;
        case 3:
            text = "1/50";
            break;
        case 4:
            text = "1/100";
            break;
        case 5:
            text = "1/250";
            break;
        case 6:
            text = "1/500";
            break;
        case 7:
            text = "1/750";
            break;
        case 8:
            text = "1/1000";
            break;
        case 9:
            text = "1/2000";
            break;
        case 10:
            text = "1/4000";
            break;
        case 11:
            text = "1/10000";
            break;
        case 12:
            text = "1/100000";
            break;
        case 13:
            text = "1";
            break;
        }
    }
    return text;
}

void ManualModeSettings::editSchedule(int exposure_time, int gain_level, int exposure_time_new, int gain_level_new)
{
    for (int day = 0; day < MAX_DAY_NUM; ++day) {
        exposure_schedule_day &schedule_day = m_exposureShce->schedule_day[day];
        for (int i = 0; i < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; ++i) {
            exposure_schedule_item &schedule_item = schedule_day.schedule_item[i];
            if (schedule_item.exposureTime == exposure_time && schedule_item.gainLevel == gain_level) {
                schedule_item.exposureTime = exposure_time_new;
                schedule_item.gainLevel = gain_level_new;
            }
        }
    }
}

void ManualModeSettings::clearSchedule(int exposure_time, int gain_level)
{
    for (int day = 0; day < MAX_DAY_NUM; ++day) {
        exposure_schedule_day &schedule_day = m_exposureShce->schedule_day[day];
        for (int i = 0; i < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; ++i) {
            exposure_schedule_item &schedule_item = schedule_day.schedule_item[i];
            if (schedule_item.exposureTime == exposure_time && schedule_item.gainLevel == gain_level) {
                memset(&schedule_item, 0, sizeof(exposure_schedule_item));
            }
        }
    }
}

bool ManualModeSettings::saveData()
{
    //保存ipc
    sendMessageOnly(REQUEST_FLAG_SET_EXPOSURE_SCHE, m_exposureShce, sizeof(struct exposure_schedule));
    return true;
}

void ManualModeSettings::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("IMAGE/37212", "Manual Mode Settings"));
    ui->pushButton_add->setText(GET_TEXT("ANPR/103026", "Add"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void ManualModeSettings::onTableItemClicked(int row, int column)
{
    switch (column) {
    case ColumnEdit: {
        EditManualMode edit(this);
        edit.setMode(EditManualMode::ModeEdit);
        int exposure_time = ui->tableView->itemData(row, ColumnExposureTime, ExposureTimeIndexRole).toInt();
        int gain_level = ui->tableView->itemIntValue(row, ColumnGainLevel);
        edit.setExposureCtrl(m_exposureCtrl);
        edit.setExposureIndex(exposure_time);
        edit.setGainLevelValue(gain_level);
        edit.setEditIndex(row);
        const int result = edit.exec();
        if (result == EditManualMode::Accepted) {
            if (exposure_time != edit.exposureTime() || gain_level != edit.gainLevel()) {
                ui->tableView->setItemText(row, ColumnExposureTime, exposureTimeString(edit.exposureTime()));
                ui->tableView->setItemData(row, ColumnExposureTime, edit.exposureTime(), ExposureTimeIndexRole);
                ui->tableView->setItemIntValue(row, ColumnGainLevel, edit.gainLevel());
                //
                exposureType tempExposure(exposure_time, gain_level);
                exposureType newExposure(edit.exposureTime(), edit.gainLevel());

                int index = m_exposureList.indexOf(tempExposure);
                m_exposureList.removeAll(tempExposure);
                m_exposureList.insert(index, newExposure);
                editSchedule(exposure_time, gain_level, edit.exposureTime(), edit.gainLevel());
            }
        }
        break;
    }
    case ColumnDelete: {
        int exposure_time = ui->tableView->itemData(row, ColumnExposureTime, ExposureTimeIndexRole).toInt();
        int gain_level = ui->tableView->itemIntValue(row, ColumnGainLevel);
        exposureType tempExposure(exposure_time, gain_level);
        m_exposureList.removeAll(tempExposure);
        clearSchedule(exposure_time, gain_level);
        updateTable();
        break;
    }
    default:
        break;
    }
}

void ManualModeSettings::onTableItemDoubleClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
}

void ManualModeSettings::on_pushButton_add_clicked()
{
    if (ui->tableView->rowCount() >= 70) {
        ShowMessageBox("Up to 70 Manual modes are supported.");
        return;
    }
    //
    EditManualMode add(this);
    add.setMode(EditManualMode::ModeAdd);
    add.setExposureCtrl(m_exposureCtrl);
    const int result = add.exec();
    if (result == EditManualMode::Accepted) {

        exposureType tempExposure;
        tempExposure.exposure.exposureTime = add.exposureTime();
        tempExposure.exposure.gainLevel = add.gainLevel();
        m_exposureList.append(tempExposure);
    }
    ui->pushButton_add->clearUnderMouse();
    ui->pushButton_add->clearFocus();
    updateTable();
}

void ManualModeSettings::on_pushButton_ok_clicked()
{
    bool hasChange = false;
    for (auto iter : m_exposureListCache) {
        if (!m_exposureList.contains(iter)) {
            hasChange = true;
        }
    }
    if (hasChange) {
        const int result = MessageBox::question(this, GET_TEXT("IMAGE/37243", "The modification will affect the related schedule, continue?"));
        if (result == MessageBox::Yes) {
            emit settingFinish();
            accept();
        } else {
            memcpy(m_exposureShce, m_exposureShceCache, sizeof(struct exposure_schedule));
            m_exposureList.clear();
            for (auto i = m_exposureListCache.begin(); i != m_exposureListCache.end(); ++i) {
                m_exposureList.append(exposureType(i->exposure.exposureTime, i->exposure.gainLevel));
            }
            updateTable();
        }
    } else {
        memcpy(m_exposureShce, m_exposureShceCache, sizeof(struct exposure_schedule));
        emit settingFinish();
        accept();
    }
}

void ManualModeSettings::on_pushButton_cancel_clicked()
{
    memcpy(m_exposureShce, m_exposureShceCache, sizeof(struct exposure_schedule));
    m_exposureList.clear();
    for (auto i = m_exposureListCache.begin(); i != m_exposureListCache.end(); ++i) {
        m_exposureList.append(exposureType(i->exposure.exposureTime, i->exposure.gainLevel));
    }
    emit settingFinish();
    reject();
}

int ManualModeSettings::channel() const
{
    return m_channel;
}

QList<ExposureManualValue> ManualModeSettings::getManualModeList()
{
    QList<ExposureManualValue> manualModeList;
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        exposure e;
        e.exposureTime = ui->tableView->itemData(i, ColumnExposureTime, ExposureTimeIndexRole).toInt();
        e.gainLevel = ui->tableView->itemIntValue(i, ColumnGainLevel);
        ExposureManualValue manualValue(ExposureActionManual, e.exposureTime, e.gainLevel);
        if (manualValue.isValid() && !manualModeList.contains(manualValue)) {
            manualModeList.append(manualValue);
        }
    }
    return manualModeList;
}

exposure_schedule *ManualModeSettings::exposureShce() const
{
    return m_exposureShce;
}

void ManualModeSettings::setExposureShce(exposure_schedule *exposureShce)
{
    if (!exposureShce) {
        return;
    }
    if (!m_exposureShce) {
        m_exposureShce = new exposure_schedule;
    }
    memset(m_exposureShce, 0, sizeof(struct exposure_schedule));
    memset(m_exposureShceCache, 0, sizeof(struct exposure_schedule));
    memcpy(m_exposureShce, exposureShce, sizeof(struct exposure_schedule));
    memcpy(m_exposureShceCache, exposureShce, sizeof(struct exposure_schedule));
}
