#include "licenseplateadd.h"
#include "ui_licenseplateadd.h"
#include "MsDevice.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "anprthread.h"

LicensePlateAdd::LicensePlateAdd(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::LicensePlateAdd)
{
    ui->setupUi(this);

    ui->comboBox_plateType->clear();
    ui->comboBox_plateType->addItem(GET_TEXT("ANPR/103039", "Black"), QString(PARAM_MS_ANPR_TYPE_BLACK));
    ui->comboBox_plateType->addItem(GET_TEXT("ANPR/103040", "White"), QString(PARAM_MS_ANPR_TYPE_WHITE));
    ui->lineEdit_licensePlate->setCheckMode(MyLineEdit::EmptyCheck);

    onLanguageChanged();
}

LicensePlateAdd::~LicensePlateAdd()
{
    delete ui;
}

void LicensePlateAdd::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("ANPR/103092", "License Plate Add"));
    ui->label_licensePlate->setText(GET_TEXT("ANPR/103035", "License Plate"));
    ui->label_plateType->setText(GET_TEXT("ANPR/103041", "Plate Type"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void LicensePlateAdd::on_pushButton_ok_clicked()
{
    const QString &strPlate = ui->lineEdit_licensePlate->text();
    const QString &strType = ui->comboBox_plateType->currentData().toString();

    if (!ui->lineEdit_licensePlate->checkValid()) {
        return;
    }

    if (strPlate.contains(" ")) {
        ui->lineEdit_licensePlate->setCustomValid(false, GET_TEXT("MYLINETIP/112012", "Invalid character: space."));
        return;
    }

    int anprListCount = read_anpr_list_cnt(SQLITE_ANPR_NAME);

    anpr_list anpr_info;
    memset(&anpr_info, 0, sizeof(anpr_list));
    read_anpr_list_plate(SQLITE_ANPR_NAME, &anpr_info, strPlate.toStdString().c_str());
    if (!QString(anpr_info.plate).isEmpty())
    {
        int result = MessageBox::question(this, GET_TEXT("ANPR/103045", "Detected the same plate(s). Do you want to replace the old ones?"), MessageBox::AlignCenter);
        if (result == MessageBox::Cancel)
        {
            reject();
            return;
        }
        else
        {
            snprintf(anpr_info.plate, sizeof(anpr_info.plate), "%s", strPlate.toStdString().c_str());
            snprintf(anpr_info.type, sizeof(anpr_info.type), "%s", strType.toStdString().c_str());
            update_anpr_list(SQLITE_ANPR_NAME, &anpr_info);
        }
    }
    else
    {
        if (anprListCount >= MAX_ANPR_LIST_COUNT)
        {
            ShowMessageBox(this, GET_TEXT("ANPR/103065", "Out of capacity！"));
            reject();
            return;
        }
        snprintf(anpr_info.plate, sizeof(anpr_info.plate), "%s", strPlate.toStdString().c_str());
        snprintf(anpr_info.type, sizeof(anpr_info.type), "%s", strType.toStdString().c_str());
        write_anpr_list(SQLITE_ANPR_NAME, &anpr_info);
    }

    accept();
}

void LicensePlateAdd::on_pushButton_cancel_clicked()
{
    reject();
}
