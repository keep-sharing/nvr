#include "licenseplateformateditor.h"
#include "ui_licenseplateformateditor.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "anprlicenseplateformatsettings.h"
#include <QDebug>
#include <cstring>

LicensePlateFormatEditor::LicensePlateFormatEditor(QWidget *parent, bool isAddingItem, int maxNum)
    : BaseShadowDialog(parent)
    , ui(new Ui::LicensePlateFormatEditor)
    , m_isAddingItem(isAddingItem)
{
    ui->setupUi(this);
    if (m_isAddingItem)
        ui->label_title->setText(GET_TEXT("ANPR/103086", "License Plate Format Add"));
    else
        ui->label_title->setText(GET_TEXT("ANPR/103087", "License Plate Format Edit"));
    for (int i = 1; i <= maxNum; i++)
        ui->comboBox->addItem(QString::number(i));
    ui->label_licensePlateCharacterCount->setText(GET_TEXT("ANPR/103083", "License Plate Character Count"));
    ui->label_licensePlateFormat->setText(GET_TEXT("ANPR/103081", "License Plate Format"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->comboBox->setCurrentIndex(0);
    ui->lineEdit_licensePlate->setText("*");
    ui->lineEdit_licensePlate->setMaxLength(1);

    onLanguageChanged();
}

LicensePlateFormatEditor::~LicensePlateFormatEditor()
{
    delete ui;
}

void LicensePlateFormatEditor::on_comboBox_indexSet(int index)
{
    Q_UNUSED(index)

    QString format;
    for (int i = 0; i < ui->comboBox->currentText().toInt(); i++) {
        format += "*";
    }
    ui->lineEdit_licensePlate->setMaxLength(format.size());
    ui->lineEdit_licensePlate->setText(format);
}

void LicensePlateFormatEditor::on_pushButton_cancel_clicked()
{
    reject();
}

void LicensePlateFormatEditor::on_pushButton_ok_clicked()
{
    if (!sanityCheck())
        return;
    accept();
}

void LicensePlateFormatEditor::setCharacterCount(int c)
{
    ui->comboBox->setCurrentIndex(c - 1);
    currentCharacterCount = c;
}

void LicensePlateFormatEditor::setFormat(const QString &format)
{
    ui->lineEdit_licensePlate->setText(format);
}

int LicensePlateFormatEditor::characterCount()
{
    return ui->comboBox->currentText().toInt();
}

QString LicensePlateFormatEditor::format()
{
    return ui->lineEdit_licensePlate->text();
}

bool LicensePlateFormatEditor::sanityCheck()
{
    bool ret;
    QObject *d = qobject_cast<AnprLicensePlateFormatSettings *>(parent())->d_func();

    if (m_isAddingItem || currentCharacterCount != characterCount()) {
        QMetaObject::invokeMethod(d, "isExistingCharacterCount", Q_RETURN_ARG(bool, ret), Q_ARG(int, characterCount()));
        if (ret) {
            ShowMessageBox(GET_TEXT("ANPR/103085", "The same character count already exists, cannot be added twice."));
            return false;
        }
    }

    if (format().isEmpty()) {
        ShowMessageBox("The license plate format does not match the license plate character count.");
        return false;
    }

    auto len = std::strspn(format().toLocal8Bit(), "A1*");
    if (int(len) != format().size()) {
        ShowMessageBox(GET_TEXT("ANPR/103084", "Only A, 1 and * are allowed in format filed."));
        return false;
    }

    if (format().size() != characterCount()) {
        ShowMessageBox("The license plate format does not match the license plate character count.");
        return false;
    }

    return true;
}

bool LicensePlateFormatEditor::isAddingItem()
{
    return m_isAddingItem;
}
