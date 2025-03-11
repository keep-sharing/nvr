#include "anprdetectionsettings.h"
#include "ui_anprdetectionsettings.h"
#include "MsCameraVersion.h"
#include "MsLanguage.h"
#include "anprlicenseplateformatsettings.h"
#include <QtDebug>

extern "C" {
#include "msdb.h"
#include "msg.h"
}

AnprDetectionSettings::AnprDetectionSettings(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::AnprDetectionSettings)
{
    ui->setupUi(this);

    ui->comboBox_detectionTrigger->clear();
    ui->comboBox_detectionTrigger->addItem(GET_TEXT("ANPR/103016", "Always"), 0);
    ui->comboBox_detectionTrigger->addItem(GET_TEXT("ANPR/103017", "Camera Alarm Input"), 1);

    ui->comboBox_checktime->clear();
    ui->comboBox_checktime->addItem(GET_TEXT("ANPR/103063", "Milliseconds"), 0);
    ui->comboBox_checktime->addItem(GET_TEXT("ANPR/103064", "Minute"), 2);

    ui->horizontalSlider_confidenceLevel->setRange(1, 10);

    m_checkBoxList.append(ui->checkBox_region);
    m_checkBoxList.append(ui->checkBox_direction);
    m_checkBoxList.append(ui->checkBox_roi_id);
    m_checkBoxList.append(ui->checkBoxPlateColor);
    m_checkBoxList.append(ui->checkBoxCountryRegion);
    m_checkBoxList.append(ui->checkBoxVehicleType);
    m_checkBoxList.append(ui->checkBoxVehicleBrand);
    m_checkBoxList.append(ui->checkBoxVehicleColor);
    for (int i = 0; i < m_checkBoxList.count(); ++i) {
        QCheckBox *checkBox = m_checkBoxList.at(i);
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(updateAllCheckState()));
    }
    connect(this, SIGNAL(needLicensePlateFormatChanged()), this, SLOT(onNeedLicensePlateFormatChanged()));
    m_shieldingList.clear();
    m_shieldingList << "South Korea"
                    << "Japan"
                    << "Thailand"
                    << "India"
                    << "China"
                    << "Taiwan"
                    << "Iraq"
                    << "Iran";

    onLanguageChanged();
}

AnprDetectionSettings::~AnprDetectionSettings()
{
    delete ui;
}

void AnprDetectionSettings::showSettings(int version, ms_lpr_settings *lpr_settings, ms_lpr_wildcards *lpr_wildcards)
{
    m_lpr_version = version;
    m_lpr_settings = lpr_settings;
    m_lpr_wildcards = lpr_wildcards;

    char channelVersion[64];
    //get_channel_version_name(lpr_settings->chanid, channelVersion, sizeof(channelVersion));
    MsCameraVersion cameraVersion(channelVersion);

    setNeedLicensePlateFormat(false);

    if (cameraVersion >= MsCameraVersion(7, 74) && !m_shieldingList.contains(QString(lpr_settings->cur_area))) {
        setNeedLicensePlateFormat(true);
    }
    ui->comboBox_detectionTrigger->setCurrentIndexFromData(m_lpr_settings->process_mode);
    ui->horizontalSlider_confidenceLevel->setValue(m_lpr_settings->confidence);
    ui->comboBox_checktime->setCurrentIndexFromData(m_lpr_settings->repeat_check_time_unit);
    ui->spinBox_checktime->setValue(m_lpr_settings->repeat_check_time_value);

    switch (m_lpr_version) {
    case PARAM_MS_ANPR_VERSION1:
        ui->label_confidenceLevel->setVisible(true);
        ui->horizontalSlider_confidenceLevel->setVisible(true);
        ui->checkBox_region->setVisible(false);
        break;
    case PARAM_MS_ANPR_VERSION2:
        ui->label_confidenceLevel->setVisible(true);
        ui->horizontalSlider_confidenceLevel->setVisible(true);
        ui->checkBox_region->setVisible(true);
        setNeedLicensePlateFormat(false);
        break;
    case PARAM_MS_ANPR_VERSION3:
        ui->label_confidenceLevel->setVisible(false);
        ui->horizontalSlider_confidenceLevel->setVisible(false);
        ui->checkBox_region->setVisible(false);
        setNeedLicensePlateFormat(false);
        break;
    }

    bool isTs = cameraVersion >= MsCameraVersion(8, 2);
    ui->checkBoxPlateColor->setVisible(isTs);
    ui->checkBoxVehicleType->setVisible(isTs);
    ui->checkBoxVehicleColor->setVisible(isTs);
    ui->checkBoxCountryRegion->setVisible(isTs);
    if (isTs) {
        switch (lpr_settings->aitype) {
        case IPC_AITYPE_LPR_AM:
        case IPC_AITYPE_LPR_AP:
        case IPC_AITYPE_LPR_EU:
        case IPC_AITYPE_LPR_ME:
        case IPC_AITYPE_LPR3:
            ui->label_confidenceLevel->setVisible(false);
            ui->horizontalSlider_confidenceLevel->setVisible(false);
            break;
        default:
            ui->label_confidenceLevel->setVisible(true);
            ui->horizontalSlider_confidenceLevel->setVisible(true);
            break;
        }
        ui->checkBox_region->setVisible(false);
    }

    ui->checkBoxVehicleBrand->setVisible(m_lpr_settings->vehicleBrandEnable == 0 || m_lpr_settings->vehicleBrandEnable == 1);

    ui->checkBox_region->setChecked(m_lpr_settings->region_enable);
    ui->checkBox_direction->setChecked(m_lpr_settings->direction_enable);
    ui->checkBox_roi_id->setChecked(m_lpr_settings->roiid_enable);

    ui->checkBoxPlateColor->setChecked(m_lpr_settings->platecolorEnable);
    ui->checkBoxVehicleType->setChecked(m_lpr_settings->vehicletypeEnable);
    ui->checkBoxVehicleColor->setChecked(m_lpr_settings->vehiclecolorEnable);
    ui->checkBoxCountryRegion->setChecked(m_lpr_settings->region_enable);
    ui->checkBoxVehicleBrand->setChecked(m_lpr_settings->vehicleBrandEnable);
}

void AnprDetectionSettings::saveSettings()
{
    m_lpr_settings->process_mode = ui->comboBox_detectionTrigger->currentData().toInt();
    m_lpr_settings->confidence = static_cast<int>(ui->horizontalSlider_confidenceLevel->value());
    m_lpr_settings->repeat_check_time_unit = ui->comboBox_checktime->currentData().toInt();
    m_lpr_settings->repeat_check_time_value = ui->spinBox_checktime->value();
    if (!ui->checkBoxCountryRegion->isVisible()) {
        m_lpr_settings->region_enable = ui->checkBox_region->isChecked();
    } else {
        m_lpr_settings->region_enable = ui->checkBoxCountryRegion->isChecked();
    }
    m_lpr_settings->direction_enable = ui->checkBox_direction->isChecked();
    m_lpr_settings->roiid_enable = ui->checkBox_roi_id->isChecked();

    m_lpr_settings->platecolorEnable = ui->checkBoxPlateColor->isChecked();
    m_lpr_settings->vehicletypeEnable = ui->checkBoxVehicleType->isChecked();
    m_lpr_settings->vehiclecolorEnable = ui->checkBoxVehicleColor->isChecked();
    m_lpr_settings->vehicleBrandEnable = ui->checkBoxVehicleBrand->isChecked();
}

void AnprDetectionSettings::showEvent(QShowEvent *event)
{
    updateAllCheckState();
    BaseShadowDialog::showEvent(event);
}

void AnprDetectionSettings::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("ANPR/103014", "Detection Settings"));
    ui->label_detectionTrigger->setText(GET_TEXT("ANPR/103015", "Detection Trigger"));
    ui->label_confidenceLevel->setText(GET_TEXT("ANPR/103018", "Confidence Level"));
    ui->label_repeatPlateChecktime->setText(GET_TEXT("ANPR/103019", "Repeat Plate Checktime"));
    ui->label_featuresIdentification->setText(GET_TEXT("ANPR/169089", "Attributes Identification"));
    ui->label_licensePlateFormat->setText(GET_TEXT("ANPR/103081", "License Plate Format"));

    ui->checkBox_all->setText(GET_TEXT("SYSTEMGENERAL/164000", "All"));
    ui->checkBox_region->setText(GET_TEXT("ANPR/103021", "Region"));
    ui->checkBox_direction->setText(GET_TEXT("ANPR/103022", "Direction"));
    ui->checkBox_roi_id->setText(GET_TEXT("ANPR/103023", "ROI_ID"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->pushButton_editLicensePlateFormat->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->checkBoxPlateColor->setText(GET_TEXT("ANPR/169002", "Plate Color"));
    ui->checkBoxVehicleType->setText(GET_TEXT("ANPR/169003", "Vehicle Type"));
    ui->checkBoxVehicleColor->setText(GET_TEXT("ANPR/169004", "Vehicle Color"));
    ui->checkBoxCountryRegion->setText(GET_TEXT("ANPR/169005", "Country/Region"));
    ui->checkBoxVehicleBrand->setText(GET_TEXT("ANPR/169006", "Vehicle Brand"));
}

void AnprDetectionSettings::updateAllCheckState()
{
    int visibleCount = 0;
    int checkedCount = 0;
    for (int i = 0; i < m_checkBoxList.count(); ++i) {
        QCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isVisible()) {
            visibleCount++;
            if (checkBox->isChecked()) {
                checkedCount++;
            }
        } else {
        }
    }
    if (checkedCount == 0) {
        ui->checkBox_all->setCheckState(Qt::Unchecked);
    } else if (checkedCount < visibleCount) {
        ui->checkBox_all->setCheckState(Qt::PartiallyChecked);
    } else {
        ui->checkBox_all->setCheckState(Qt::Checked);
    }
}

void AnprDetectionSettings::on_comboBox_checktime_currentIndexChanged(int index)
{
    int data = ui->comboBox_checktime->itemData(index).toInt();
    switch (data) {
    case 0:
        ui->spinBox_checktime->setRange(0, 60000);
        ui->label_checktime->setText("(0~60000ms)");
        break;
    case 2:
        ui->spinBox_checktime->setRange(0, 60);
        ui->label_checktime->setText("(0~60min)");
        break;
    }
}

void AnprDetectionSettings::on_checkBox_all_clicked(bool checked)
{
    if (checked) {
        ui->checkBox_all->setCheckState(Qt::Checked);
    }

    for (int i = 0; i < m_checkBoxList.count(); ++i) {
        QCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isVisible()) {
            checkBox->setChecked(checked);
        }
    }
}

void AnprDetectionSettings::on_pushButton_ok_clicked()
{
    saveSettings();
    accept();
}

void AnprDetectionSettings::on_pushButton_cancel_clicked()
{
    reject();
}
bool AnprDetectionSettings::needLicensePlateFormat()
{
    return m_needLicensePlateFormat;
}

void AnprDetectionSettings::setNeedLicensePlateFormat(bool b)
{
    if (m_needLicensePlateFormat == b)
        return;
    m_needLicensePlateFormat = b;
    emit needLicensePlateFormatChanged();
}

void AnprDetectionSettings::mayDisplayLicensePlateFormatPanel()
{
    ui->label_licensePlateFormat->setVisible(needLicensePlateFormat());
    ui->widgetLicensePlateFormat->setVisible(needLicensePlateFormat());
}

void AnprDetectionSettings::onNeedLicensePlateFormatChanged()
{
    mayDisplayLicensePlateFormatPanel();
}

void AnprDetectionSettings::on_pushButton_editLicensePlateFormat_clicked()
{
    qDebug() << __FUNCTION__;
    AnprLicensePlateFormatSettings settings(this, m_lpr_wildcards);
    settings.exec();
}
