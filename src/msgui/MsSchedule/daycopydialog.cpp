#include "daycopydialog.h"
#include "ui_daycopydialog.h"
#include "MsLanguage.h"

DayCopyDialog::DayCopyDialog(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::DayCopyDialog)
{
    ui->setupUi(this);

    m_checkBoxList.append(ui->checkBox_0);
    m_checkBoxList.append(ui->checkBox_1);
    m_checkBoxList.append(ui->checkBox_2);
    m_checkBoxList.append(ui->checkBox_3);
    m_checkBoxList.append(ui->checkBox_4);
    m_checkBoxList.append(ui->checkBox_5);
    m_checkBoxList.append(ui->checkBox_6);
    m_checkBoxList.append(ui->checkBox_7);
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        QCheckBox *checkBox = m_checkBoxList.at(i);
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxClicked()));
    }

    onLanguageChanged();
}

DayCopyDialog::~DayCopyDialog()
{
    delete ui;
}

void DayCopyDialog::setCurrentDay(int day)
{
    QCheckBox *checkBox = m_checkBoxList.at(day);
    checkBox->setChecked(true);
    checkBox->setEnabled(false);
}

QList<int> DayCopyDialog::dayList() const
{
    QList<int> list;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        QCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked() && checkBox->isEnabled()) {
            list.append(i);
        }
    }
    return list;
}

void DayCopyDialog::setHolidayVisible(bool visible)
{
    ui->checkBox_7->setVisible(visible);
}

void DayCopyDialog::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("COMMON/1058", "Day Copy"));
    ui->checkBox_all->setText(GET_TEXT("SYSTEMGENERAL/164000", "All"));
    ui->checkBox_0->setText(GET_TEXT("COMMON/1024", "Sunday"));
    ui->checkBox_1->setText(GET_TEXT("COMMON/1025", "Monday"));
    ui->checkBox_2->setText(GET_TEXT("COMMON/1026", "Tuesday"));
    ui->checkBox_3->setText(GET_TEXT("COMMON/1027", "Wednesday"));
    ui->checkBox_4->setText(GET_TEXT("COMMON/1028", "Thursday"));
    ui->checkBox_5->setText(GET_TEXT("COMMON/1029", "Friday"));
    ui->checkBox_6->setText(GET_TEXT("COMMON/1030", "Saturday"));
    ui->checkBox_7->setText(GET_TEXT("COMMON/1031", "Holiday"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void DayCopyDialog::onCheckBoxClicked()
{
    int checkedCount = 0;
    int disableCount = 0;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        QCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked() && checkBox->isEnabled() && checkBox->isVisible()) {
            checkedCount++;
        }
        if (!checkBox->isEnabled() || !checkBox->isVisible()) {
            disableCount++;
        }
    }
    if (checkedCount == 0) {
        ui->checkBox_all->setCheckState(Qt::Unchecked);
    } else if (checkedCount == m_checkBoxList.size() - disableCount) {
        ui->checkBox_all->setCheckState(Qt::Checked);
    } else {
        ui->checkBox_all->setCheckState(Qt::PartiallyChecked);
    }
}

void DayCopyDialog::on_checkBox_all_clicked(bool checked)
{
    if (checked) {
        ui->checkBox_all->setCheckState(Qt::Checked);
    }
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        QCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isEnabled()) {
            checkBox->setChecked(checked);
        }
    }
}

void DayCopyDialog::on_pushButton_ok_clicked()
{
    accept();
}

void DayCopyDialog::on_pushButton_cancel_clicked()
{
    reject();
}
