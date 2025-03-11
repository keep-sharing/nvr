#include "camerachanneladd.h"
#include "ui_camerachanneladd.h"
#include "MsGlobal.h"
#include "MsLanguage.h"

CameraChannelAdd::CameraChannelAdd(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::CameraChannelAdd)
{
    ui->setupUi(this);

    m_allCheckBox.append(ui->checkBox_1);
    m_allCheckBox.append(ui->checkBox_2);
    m_allCheckBox.append(ui->checkBox_3);
    m_allCheckBox.append(ui->checkBox_4);
    m_allCheckBox.append(ui->checkBox_5);
    for (int i = 0; i < 5; ++i) {
        QCheckBox *checkBox = m_allCheckBox.at(i);
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onCheckboxClicked()));
    }

    onLanguageChanged();
}

CameraChannelAdd::~CameraChannelAdd()
{
    delete ui;
}

int CameraChannelAdd::execAdd(const QMap<int, int> &map, int count)
{
    m_realCheckBoxList.clear();

    for (int i = 0; i < 5; ++i) {
        QCheckBox *checkBox = m_allCheckBox.at(i);
        if (i < count) {
            checkBox->show();
            m_realCheckBoxList.append(checkBox);
        } else {
            checkBox->hide();
        }
        if (map.contains(i + 1)) {
            checkBox->setChecked(true);
            checkBox->setEnabled(true);
        } else {
            checkBox->setChecked(true);
            checkBox->setEnabled(false);
        }
    }
    onCheckboxClicked();
    return exec();
}

QList<int> CameraChannelAdd::addList() const
{
    return m_addList;
}

void CameraChannelAdd::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("FISHEYE/12012", "Camera Channel Add"));
    ui->label_channelId->setText(GET_TEXT("FISHEYE/12016", "Channel ID"));
    ui->checkBox_all->setText(GET_TEXT("SYSTEMGENERAL/164000", "All"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void CameraChannelAdd::onCheckboxClicked()
{
    int checkedCount = 0;
    for (int i = 0; i < m_realCheckBoxList.size(); ++i) {
        QCheckBox *checkBox = m_realCheckBoxList.at(i);
        if (checkBox->isChecked()) {
            checkedCount++;
        }
    }
    if (checkedCount == 0) {
        ui->checkBox_all->setCheckState(Qt::Unchecked);
    } else if (checkedCount >= m_realCheckBoxList.size()) {
        ui->checkBox_all->setCheckState(Qt::Checked);
    } else {
        ui->checkBox_all->setCheckState(Qt::PartiallyChecked);
    }
}

void CameraChannelAdd::on_checkBox_all_clicked(bool checked)
{
    for (int i = 0; i < m_realCheckBoxList.size(); ++i) {
        QCheckBox *checkBox = m_realCheckBoxList.at(i);
        if (checkBox->isVisible() && checkBox->isEnabled()) {
            checkBox->setChecked(checked);
        }
    }
}

void CameraChannelAdd::on_pushButton_ok_clicked()
{
    m_addList.clear();
    for (int i = 0; i < m_realCheckBoxList.size(); ++i) {
        QCheckBox *checkBox = m_realCheckBoxList.at(i);
        if (checkBox->isVisible() && checkBox->isEnabled() && checkBox->isChecked()) {
            m_addList.append(i + 1);
        }
    }
    if (m_addList.isEmpty()) {
        ShowMessageBox(GET_TEXT("FISHEYE/12018", "Please select at least one channel."));
        return;
    }
    accept();
}

void CameraChannelAdd::on_pushButton_cancel_clicked()
{
    reject();
}
