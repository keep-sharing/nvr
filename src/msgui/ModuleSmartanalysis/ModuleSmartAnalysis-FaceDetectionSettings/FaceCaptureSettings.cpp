#include "FaceCaptureSettings.h"
#include "ui_FaceCaptureSettings.h"
#include "MsLanguage.h"

FaceCaptureSettings::FaceCaptureSettings(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::FaceCaptureSettings)
{
    ui->setupUi(this);

    ui->comboBoxTarger->beginEdit();
    ui->comboBoxTarger->clear();
    ui->comboBoxTarger->addItem(GET_TEXT("FACE/141014", "Face Only"), 0);
    ui->comboBoxTarger->addItem(GET_TEXT("FACE/141015", "Upper Body"), 1);
    ui->comboBoxTarger->addItem(GET_TEXT("FACE/141016", "Whole Body"), 2);
    ui->comboBoxTarger->endEdit();

    ui->comboBoxBackground->beginEdit();
    ui->comboBoxBackground->clear();
    ui->comboBoxBackground->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxBackground->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxBackground->endEdit();

    ui->comboBoxSnapshot->beginEdit();
    ui->comboBoxSnapshot->clear();
    ui->comboBoxSnapshot->addItem("1", 1);
    ui->comboBoxSnapshot->addItem("2", 2);
    ui->comboBoxSnapshot->addItem("3", 3);
    ui->comboBoxSnapshot->addItem("4", 4);
    ui->comboBoxSnapshot->addItem("5", 5);
    ui->comboBoxSnapshot->endEdit();

    ui->comboBoxSnapshotInterval->beginEdit();
    ui->comboBoxSnapshotInterval->clear();
    ui->comboBoxSnapshotInterval->addItem(GET_TEXT("FACE/141020", "80 milliseconds"), 80);
    ui->comboBoxSnapshotInterval->addItem(GET_TEXT("FACE/141021", "200 milliseconds"), 200);
    ui->comboBoxSnapshotInterval->addItem(GET_TEXT("FACE/141022", "500 milliseconds"), 500);
    ui->comboBoxSnapshotInterval->addItem(GET_TEXT("FACE/141023", "1 second"), 1000);
    ui->comboBoxSnapshotInterval->addItem(GET_TEXT("FACE/141024", "2 seconds"), 2000);
    ui->comboBoxSnapshotInterval->addItem(GET_TEXT("FACE/141025", "4 seconds"), 4000);
    ui->comboBoxSnapshotInterval->endEdit();

    ui->lineEditSide->setCheckMode(MyLineEdit::RangeCheck, 0, 180);
    ui->lineEditOblique->setCheckMode(MyLineEdit::RangeCheck, 0, 180);
    ui->lineEditPitching->setCheckMode(MyLineEdit::RangeCheck, 0, 180);

    ui->horizontalSliderCaptureQuality->setRange(1, 100);
    ui->horizontalSliderBlurLimit->setRange(1, 10);
    onLanguageChanged();
}

FaceCaptureSettings::~FaceCaptureSettings()
{
    delete ui;
}

void FaceCaptureSettings::initializeData(MsFaceConfig *faceConfig)
{
    if (!faceConfig) {
        qWarning() << "data is null.";
        return;
    }
    memset(&m_faceConfig, 0, sizeof(MsFaceConfig));
    memcpy(&m_faceConfig, faceConfig, sizeof(MsFaceConfig));

    ui->comboBoxCaptureMode->clear();
    ui->comboBoxCaptureMode->addItem(GET_TEXT("FACE/141009", "Quality Priority"), 0);
    ui->comboBoxCaptureMode->addItem(GET_TEXT("FACE/141010", "Timeliness Priority"), 1);
    if (m_faceConfig.isFaceCustomizeModeExists == MS_TRUE) {
        ui->comboBoxCaptureMode->addItem(GET_TEXT("FACE/141011", "Customize"), 2);
    }

    ui->comboBoxCaptureMode->setCurrentIndexFromData(m_faceConfig.captureMode);
    ui->horizontalSliderCaptureQuality->setValue(m_faceConfig.captureQuality[m_faceConfig.captureMode]);
    ui->comboBoxTarger->setCurrentIndexFromData(m_faceConfig.snapshotType);
    ui->comboBoxBackground->setCurrentIndexFromData(m_faceConfig.snapshotBackground);
    ui->comboBoxSnapshot->setCurrentIndexFromData(m_faceConfig.rule.snapshot[m_faceConfig.captureMode]);
    ui->comboBoxSnapshotInterval->setCurrentIndexFromData(m_faceConfig.snapshotInterval);
    ui->lineEditOblique->setText(QString("%1").arg(m_faceConfig.poseRoll));
    ui->lineEditSide->setText(QString("%1").arg(m_faceConfig.poseYaw));
    ui->lineEditPitching->setText(QString("%1").arg(m_faceConfig.posePitch));
    ui->horizontalSliderBlurLimit->setValue(m_faceConfig.blurLimit);
}

void FaceCaptureSettings::updateVisible(bool enable)
{
    ui->labelSnapshotInterval->setVisible(enable);
    ui->comboBoxSnapshotInterval->setVisible(enable);
    ui->labelOblique->setVisible(enable);
    ui->widgetOblique->setVisible(enable);
    ui->labelPitching->setVisible(enable);
    ui->widgetPitching->setVisible(enable);
    ui->labelSide->setVisible(enable);
    ui->widgetSide->setVisible(enable);
    ui->labelBlurLimit->setVisible(enable);
    ui->horizontalSliderBlurLimit->setVisible(enable);
}

void FaceCaptureSettings::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("FACE/141007", "Face Capture Settings"));
    ui->labelCaptureMode->setText(GET_TEXT("FACE/141008", "Capture Mode"));
    ui->labelCaptureQuality->setText(GET_TEXT("FACE/141012", "Capture Quality"));
    ui->labelSnapshotInterval->setText(GET_TEXT("FACE/141019", "Snapshot Interval"));
    ui->labelOblique->setText(GET_TEXT("FACE/141026", "Oblique Face Angle Limit"));
    ui->labelPitching->setText(GET_TEXT("FACE/141027", "Pitching Face Angle Limit"));
    ui->labelSide->setText(GET_TEXT("FACE/141028", "Side Face Angle Limit"));
    ui->labelBlurLimit->setText(GET_TEXT("FACE/141029", "Blur Limit"));
    ui->labelTarget->setText(GET_TEXT("FACE/141013", "Target Snapshot Type"));
    ui->labelBackground->setText(GET_TEXT("FACE/141017", "Background Snapshot"));
    ui->labelSnapshot->setText(GET_TEXT("FACE/141018", "Snapshot"));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void FaceCaptureSettings::on_comboBoxCaptureMode_indexSet(int index)
{
    if (m_faceConfig.captureMode == 2) {
        if (!lineChecked()) {
            ui->comboBoxCaptureMode->beginEdit();
            ui->comboBoxCaptureMode->setCurrentIndex(2);
            ui->comboBoxCaptureMode->endEdit();
            return;
        }
    }
    int indexData = ui->comboBoxCaptureMode->itemData(index).toInt();
    switch (indexData) {
    case 0:
        ui->comboBoxSnapshot->setEnabled(false);
        updateVisible(false);
        break;
    case 1:
        ui->comboBoxSnapshot->setEnabled(true);
        updateVisible(false);
        break;
    case 2:
        ui->comboBoxSnapshot->setEnabled(true);
        updateVisible(true);
    }
    m_faceConfig.captureMode = indexData;
    ui->horizontalSliderCaptureQuality->setValue(m_faceConfig.captureQuality[m_faceConfig.captureMode]);
    ui->comboBoxSnapshot->setCurrentIndexFromData(m_faceConfig.rule.snapshot[m_faceConfig.captureMode]);
}

void FaceCaptureSettings::on_pushButtonOk_clicked()
{
    if (!lineChecked()) {
        return;
    }
    m_faceConfig.snapshotInterval = ui->comboBoxSnapshotInterval->currentIntData();
    m_faceConfig.poseYaw = ui->lineEditSide->text().toInt();
    m_faceConfig.poseRoll = ui->lineEditOblique->text().toInt();
    m_faceConfig.posePitch = ui->lineEditPitching->text().toInt();
    m_faceConfig.blurLimit = static_cast<int>(ui->horizontalSliderBlurLimit->value());
    m_faceConfig.snapshotType = ui->comboBoxTarger->currentIntData();
    m_faceConfig.snapshotBackground = ui->comboBoxBackground->currentIntData();

    accept();
}

void FaceCaptureSettings::on_pushButtonCancel_clicked()
{
    reject();
}

void FaceCaptureSettings::on_comboBoxSnapshot_activated(int index)
{
    Q_UNUSED(index);
    m_faceConfig.rule.snapshot[m_faceConfig.captureMode] = ui->comboBoxSnapshot->currentIntData();
}

void FaceCaptureSettings::on_horizontalSliderCaptureQuality_valueChanged(int value)
{
    m_faceConfig.captureQuality[m_faceConfig.captureMode] = value;
}

bool FaceCaptureSettings::lineChecked()
{
    bool isValid = ui->lineEditSide->checkValid();
    isValid &= ui->lineEditOblique->checkValid();
    isValid &= ui->lineEditPitching->checkValid();
    return isValid;
}

MsFaceConfig *FaceCaptureSettings::faceConfig()
{
    return &m_faceConfig;
}
