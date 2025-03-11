#include "FaceAttributeRecognitionSettings.h"
#include "ui_FaceAttributeRecognitionSettings.h"
#include "MsLanguage.h"

FaceAttributeRecognitionSettings::FaceAttributeRecognitionSettings(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::FaceAttributeRecognitionSettings)
{
    ui->setupUi(this);
    m_checkBoxMap.insert(0, ui->checkBoxAge);
    m_checkBoxMap.insert(1, ui->checkBoxGender);
    m_checkBoxMap.insert(2, ui->checkBoxGlasses);
    m_checkBoxMap.insert(3, ui->checkBoxMask);
    m_checkBoxMap.insert(4, ui->checkBoxCap);

    ui->comboBoxRecognition->beginEdit();
    ui->comboBoxRecognition->clear();
    ui->comboBoxRecognition->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxRecognition->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxRecognition->endEdit();

    for (auto iter = m_checkBoxMap.constBegin(); iter != m_checkBoxMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxClicked(bool)));
    }
    onLanguageChanged();
}

FaceAttributeRecognitionSettings::~FaceAttributeRecognitionSettings()
{
    delete ui;
}

void FaceAttributeRecognitionSettings::initializeData(int attributeEnable, int attributeType)
{
    ui->comboBoxRecognition->setCurrentIndexFromData(attributeEnable);
    for (int i = 0; i < 5; i++) {
        QCheckBox *checkBox = m_checkBoxMap.value(i);
        int checked = (1 << i) & attributeType;
        checkBox->setChecked(checked);
    }
    int data = ui->comboBoxRecognition->currentIntData();
    bool enabled = static_cast<bool>(data);
    ui->widgetAttribute->setEnabled(ui->comboBoxRecognition->isEnabled() && enabled);
    onCheckBoxClicked(true);
    switch (m_note) {
    case TypeModeError:
        ui->labelNote->show();
        ui->labelNote->setText(GET_TEXT("FACE/141064", "Note: Available when Capture Mode is Quality Priority."));
        break;
    case TypeFacePrivacyEnable:
        ui->labelNote->show();
        ui->labelNote->setText(GET_TEXT("FACE/141063", "Note: Available when Face Privacy Mode is disabled."));
        break;
    case TypeNone:
        ui->labelNote->hide();
        break;
    }
}

void FaceAttributeRecognitionSettings::onLanguageChanged()
{
    ui->checkBoxAll->setText(GET_TEXT("SYSTEMGENERAL/164000", "All"));
    ui->checkBoxGender->setText(GET_TEXT("FACE/141034", "Gender"));
    ui->checkBoxGlasses->setText(GET_TEXT("FACE/141037", "Glasses"));
    ui->checkBoxAge->setText(GET_TEXT("FACE/141035", "Age"));
    ui->checkBoxMask->setText(GET_TEXT("FACE/141038", "Mask"));
    ui->checkBoxCap->setText(GET_TEXT("FACE/141039", "Cap"));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    ui->label_title->setText(GET_TEXT("FACE/141032", "Attribute Recognition Settings"));
    ui->labelAttribute->setText(GET_TEXT("FACE/141067", "Attribute"));
    ui->labelRecognition->setText(GET_TEXT("FACE/141066", "Attribute Recognition"));
}

void FaceAttributeRecognitionSettings::setIsEnabled(bool enabled)
{
    ui->comboBoxRecognition->setEnabled(enabled);
    ui->widgetAttribute->setEnabled(enabled);
}

int FaceAttributeRecognitionSettings::getAttribute()
{
    return ui->comboBoxRecognition->currentIntData();
}

int FaceAttributeRecognitionSettings::getAttributeType()
{
    int type = 0;
    for (int i = 0; i < 5; i++) {
        QCheckBox *checkBox = m_checkBoxMap.value(i);
        int checked = static_cast<int>(checkBox->isChecked());
        type |= (checked << i);
        checkBox->setChecked(checked);
    }
    return type;
}

void FaceAttributeRecognitionSettings::on_pushButtonOk_clicked()
{
    accept();
}

void FaceAttributeRecognitionSettings::on_pushButtonCancel_clicked()
{
    reject();
}

void FaceAttributeRecognitionSettings::onCheckBoxClicked(bool checked)
{
    Q_UNUSED(checked)

    int checkedCount = 0;
    for (auto iter = m_checkBoxMap.constBegin(); iter != m_checkBoxMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();

        if (checkBox->isChecked()) {
            checkedCount++;
            checkBox->setChecked(true);
        }
    }
    if (checkedCount == 0) {
        ui->checkBoxAll->setCheckState(Qt::Unchecked);
    } else if (checkedCount == 5) {
        ui->checkBoxAll->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxAll->setCheckState(Qt::PartiallyChecked);
    }
}

void FaceAttributeRecognitionSettings::on_checkBoxAll_clicked(bool checked)
{
    for (auto iter = m_checkBoxMap.constBegin(); iter != m_checkBoxMap.constEnd(); ++iter) {
        QCheckBox *checkBox = iter.value();
        if (checked) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }
    }
    if (checked) {
        ui->checkBoxAll->setCheckState(Qt::Checked);
    }
}

void FaceAttributeRecognitionSettings::on_comboBoxRecognition_currentIndexChanged(int index)
{
    int data = ui->comboBoxRecognition->itemData(index).toInt();
    bool enabled = static_cast<bool>(data);
    ui->widgetAttribute->setEnabled(ui->comboBoxRecognition->isEnabled() && enabled);
}

void FaceAttributeRecognitionSettings::setNote(const NoteType &note)
{
    m_note = note;
}

void FaceAttributeRecognitionSettings::clear()
{
    m_note = TypeNone;
}
