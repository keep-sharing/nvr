#include "TargetPreviewSettings.h"
#include "ui_TargetPreviewSettings.h"
#include "MsLanguage.h"
#include "TargetInfoManager.h"
#include <QFile>

extern "C" {
#include "msdb.h"
}

TargetPreviewSettings::TargetPreviewSettings(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::TargetPreviewSettings)
{
    ui->setupUi(this);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    }
    file.close();

    ui->label_title->setText(GET_TEXT("TARGETMODE/103208", "Target Preview Settings"));
    ui->labelAnpr->setText(GET_TEXT("ANPR/103005", "ANPR"));
    ui->labelFace->setText(GET_TEXT("FACE/141000", "Face Detection"));
    ui->labelVca->setText(GET_TEXT("SMARTEVENT/55000", "VCA"));

    ui->comboBoxAnpr->addItem(GET_TEXT("TARGETMODE/103203", "Display"), TargetInfo::TARGET_ANPR);
    ui->comboBoxAnpr->addItem(GET_TEXT("TARGETMODE/103204", "Hide"), 0);
    ui->comboBoxVca->addItem(GET_TEXT("TARGETMODE/103203", "Display"), TargetInfo::TARGET_VCA);
    ui->comboBoxVca->addItem(GET_TEXT("TARGETMODE/103204", "Hide"), 0);
    ui->comboBoxFace->addItem(GET_TEXT("TARGETMODE/103203", "Display"), TargetInfo::TARGET_FACE);
    ui->comboBoxFace->addItem(GET_TEXT("TARGETMODE/103204", "Hide"), 0);

    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    int value = get_param_int(SQLITE_FILE_NAME, "target_preview_diaplay", 0);
    ui->comboBoxAnpr->setCurrentIndexFromData(value & TargetInfo::TARGET_ANPR);
    ui->comboBoxVca->setCurrentIndexFromData(value & TargetInfo::TARGET_VCA);
    ui->comboBoxFace->setCurrentIndexFromData(value & TargetInfo::TARGET_FACE);
}

TargetPreviewSettings::~TargetPreviewSettings()
{
    delete ui;
}

void TargetPreviewSettings::on_pushButtonOk_clicked()
{
    int value = 0;
    if (ui->comboBoxAnpr->currentData().toInt() == TargetInfo::TARGET_ANPR) {
        value |= TargetInfo::TARGET_ANPR;
    }
    if (ui->comboBoxVca->currentData().toInt() == TargetInfo::TARGET_VCA) {
        value |= TargetInfo::TARGET_VCA;
    }
    if (ui->comboBoxFace->currentData().toInt() == TargetInfo::TARGET_FACE) {
        value |= TargetInfo::TARGET_FACE;
    }
    set_param_int(SQLITE_FILE_NAME, "target_preview_diaplay", value);

    gTargetInfoManager.setDisplayFlags(value);
    close();
}

void TargetPreviewSettings::on_pushButtonCancel_clicked()
{
    close();
}
