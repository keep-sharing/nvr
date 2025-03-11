#include "licenseplateedit.h"
#include "ui_licenseplateedit.h"
#include <QtDebug>
#include "MsDevice.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "anprthread.h"

LicensePlateEdit::LicensePlateEdit(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::LicensePlateEdit)
{
    ui->setupUi(this);

    ui->comboBox_plateType->clear();
    ui->comboBox_plateType->addItem(GET_TEXT("ANPR/103039", "Black"), PARAM_MS_ANPR_TYPE_BLACK);
    ui->comboBox_plateType->addItem(GET_TEXT("ANPR/103040", "White"), PARAM_MS_ANPR_TYPE_WHITE);

    ui->label_title->setText(GET_TEXT("ANPR/103047", "License Plate Edit"));
    ui->label_licensePlate->setText(GET_TEXT("ANPR/103035", "License Plate"));
    ui->label_plateType->setText(GET_TEXT("ANPR/103041", "Plate Type"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->lineEdit_licensePlate->setCheckMode(MyLineEdit::EmptyCheck);
}

LicensePlateEdit::~LicensePlateEdit()
{
    delete ui;
}

void LicensePlateEdit::setLicensePlate(const QString &plate, const QString &type)
{
    //qDebug() << QString("LicensePlateEdit::setLicensePlate") << type << ui->comboBox_plateType->findData(type);
    m_oldPlate = plate;
    m_oldType = type;
    ui->lineEdit_licensePlate->setText(plate);
    ui->comboBox_plateType->setCurrentIndexFromData(type);
}

void LicensePlateEdit::on_pushButton_ok_clicked()
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

    anpr_list anpr_info;
    snprintf(anpr_info.plate, sizeof(anpr_info.plate), "%s", strPlate.toStdString().c_str());
    snprintf(anpr_info.type, sizeof(anpr_info.type), "%s", strType.toStdString().c_str());

    if (strPlate == m_oldPlate)
    {
        update_anpr_list(SQLITE_ANPR_NAME, &anpr_info);
        accept();
        return;
    }

    anpr_list temp_anpr_info;
    memset(&temp_anpr_info, 0, sizeof(anpr_list));
    read_anpr_list_plate(SQLITE_ANPR_NAME, &temp_anpr_info, strPlate.toStdString().c_str());
    if (!QString(temp_anpr_info.plate).isEmpty())
    {
        int result = MessageBox::question(this, GET_TEXT("ANPR/103045", "Detected the same plate(s). Do you want to replace the old ones?"), MessageBox::AlignCenter);
        if (result == MessageBox::Cancel)
        {
            reject();
            return;
        }
        else
        {
            update_anpr_list(SQLITE_ANPR_NAME, &anpr_info);
        }
    }
    else
    {
        int anprListCount = read_anpr_list_cnt(SQLITE_ANPR_NAME);
        if (anprListCount > MAX_ANPR_LIST_COUNT)
        {
            ShowMessageBox(this, GET_TEXT("ANPR/103065", "Out of capacity！"));
            return;
        }

        anpr_list old_anpr_info;
        memset(&old_anpr_info, 0, sizeof(anpr_list));
        snprintf(old_anpr_info.plate, sizeof(old_anpr_info.plate), "%s", m_oldPlate.toStdString().c_str());
        snprintf(old_anpr_info.type, sizeof(old_anpr_info.type), "%s", m_oldType.toStdString().c_str());
        delete_anpr_list(SQLITE_ANPR_NAME, &old_anpr_info);
        //
        write_anpr_list(SQLITE_ANPR_NAME, &anpr_info);
    }

    accept();
}

void LicensePlateEdit::on_pushButton_cancel_clicked()
{
    reject();
}
