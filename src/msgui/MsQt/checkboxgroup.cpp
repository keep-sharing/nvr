#include "checkboxgroup.h"
#include "ui_checkboxgroup.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "qmath.h"

extern "C" {

}

CheckBoxGroup::CheckBoxGroup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CheckBoxGroup)
{
    ui->setupUi(this);
    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

CheckBoxGroup::~CheckBoxGroup()
{
    delete ui;
}

void CheckBoxGroup::setCount(int allCount, int columnCount)
{
    m_columnCount = columnCount;
    clearCheckBoxList();
    //
    int channel = 0;
    int row = qCeil((qreal)allCount / m_columnCount) + 1;
    for (int r = 1; r < row; ++r) {
        for (int c = 0; c < m_columnCount; ++c) {
            if (m_checkBoxList.size() < allCount) {
                ChannelCheckBox *checkBox = new ChannelCheckBox(this);
                checkBox->setChannel(channel);
                connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxClicked()));
                connect(checkBox, SIGNAL(clicked(int, bool)), this, SIGNAL(checkBoxClicked(int, bool)));
                checkBox->setText(QString::number((r - 1) * m_columnCount + c + 1));
                ui->gridLayout->addWidget(checkBox, r, c);
                m_checkBoxList.append(checkBox);
                channel++;
            } else {
                break;
            }
        }
    }
}

void CheckBoxGroup::setCountFromChannelName(int allCount, int columnCount)
{
    if (allCount < columnCount) {
        columnCount = allCount;
    }
    m_columnCount = columnCount;
    clearCheckBoxList();

    int channel = 0;
    int row = qCeil((qreal)allCount / m_columnCount) + 1;
    for (int r = 1; r < row; ++r) {
        for (int c = 0; c < m_columnCount; ++c) {
            if (m_checkBoxList.size() < allCount) {
                ChannelCheckBox *checkBox = new ChannelCheckBox(this);
                checkBox->setChannel(channel);
                connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxClicked()));
                connect(checkBox, SIGNAL(clicked(int, bool)), this, SIGNAL(checkBoxClicked(int, bool)));
                checkBox->setMaximumWidth(columnCount == 4 ? 299 : 143);
                QString channelName = qMsNvr->channelName(channel);
                QFontMetrics fontWidth(this->font());
                QString elidnote = fontWidth.elidedText(channelName, Qt::ElideRight, columnCount == 4 ? 234 : 78);
                checkBox->setText(elidnote);
                checkBox->setToolTip(channelName);
                ui->gridLayout->addWidget(checkBox, r, c);
                m_checkBoxList.append(checkBox);
                channel++;
            } else {
                break;
            }
        }
    }
}

void CheckBoxGroup::setCountFromPosName(int allCount, int columnCount)
{
    if (allCount < columnCount) {
        columnCount = allCount;
    }
    m_columnCount = columnCount;
    clearCheckBoxList();

    Db_POS_CONFIG posConfigs[MAX_POS_CLIENT];
    memset(posConfigs, 0, sizeof(posConfigs));
    read_pos_settings(SQLITE_FILE_NAME, posConfigs);

    int id = 0;
    int row = qCeil((qreal)allCount / m_columnCount) + 1;
    for (int r = 1; r < row; ++r) {
        for (int c = 0; c < m_columnCount; ++c) {
            if (m_checkBoxList.size() < allCount) {
                ChannelCheckBox *checkBox = new ChannelCheckBox(this);
                checkBox->setChannel(id);
                connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxClicked()));
                connect(checkBox, SIGNAL(clicked(int, bool)), this, SIGNAL(checkBoxClicked(int, bool)));
                auto &config = posConfigs[id];
                checkBox->setMaximumWidth(columnCount == 4 ? 299 : 143);
                QString posName = config.name;
                QFontMetrics fontWidth(this->font());
                QString elidnote = fontWidth.elidedText(posName, Qt::ElideRight, columnCount == 4 ? 234 : 78);
                checkBox->setText(elidnote);
                checkBox->setToolTip(posName);
                ui->gridLayout->addWidget(checkBox, r, c);
                m_checkBoxList.append(checkBox);
                id++;
            } else {
                break;
            }
        }
    }
}

void CheckBoxGroup::setCountFromGroupName(int allCount, int columnCount)
{
    Q_UNUSED(allCount)
    m_columnCount = columnCount;
}

void CheckBoxGroup::clearCheck()
{
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        checkBox->setChecked(false);
    }
    ui->checkBoxAll->setChecked(false);
}

bool CheckBoxGroup::hasChannelSelected()
{
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isVisible() && checkBox->isEnabled() && checkBox->isChecked()) {
            return true;
        }
    }
    return false;
}

void CheckBoxGroup::setCheckedFromString(const QString &text)
{
    for (int i = 0; i < text.size(); ++i) {
        ChannelCheckBox *checkBox = nullptr;
        if (i < m_checkBoxList.size()) {
            checkBox = m_checkBoxList.at(i);
        }
        //
        if (checkBox) {
            if (text.at(i) == QChar('1')) {
                checkBox->setChecked(true);
            } else {
                checkBox->setChecked(false);
            }
        }
    }
    onCheckBoxClicked();
}

void CheckBoxGroup::setCheckedFromInt(const quint32 value)
{
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if ((value >> i) & 0x01) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }
    }
    onCheckBoxClicked();
}

void CheckBoxGroup::setAllChecked()
{
    ui->checkBoxAll->setChecked(true);
    on_checkBoxAll_clicked(true);
}

QList<bool> CheckBoxGroup::checkStateList()
{
    QList<bool> list;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        list.append(checkBox->isChecked());
    }
    return list;
}

void CheckBoxGroup::setIndexEnabled(int index, bool enable)
{
    if (m_checkBoxList.size() > index) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(index);
        checkBox->setChecked(true);
        checkBox->setEnabled(enable);
    }
}

QList<int> CheckBoxGroup::checkedList(bool containsCurrent /* = true*/) const
{
    QList<int> list;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            if (checkBox->isEnabled()) {
                list.append(i);
            } else {
                if (containsCurrent) {
                    list.append(i);
                }
            }
        }
    }
    return list;
}

QList<int> CheckBoxGroup::checkedList(int maxChecked) const
{
    int count = 0;
    QList<int> list;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            list.append(i);
            count++;
            if (count >= maxChecked) {
                break;
            }
        }
    }
    return list;
}

QString CheckBoxGroup::checkedMask() const
{
    QString text;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            text.append("1");
        } else {
            text.append("0");
        }
    }
    return text;
}

QString CheckBoxGroup::checkedMask(int maxChecked) const
{
    int count = 0;
    QString text;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked() && count < maxChecked) {
            text.append("1");
            count++;
        } else {
            text.append("0");
        }
    }
    return text;
}

quint64 CheckBoxGroup::checkedFlags(bool containsCurrent /* = true*/) const
{
    quint64 flag = 0;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            if (checkBox->isEnabled()) {
                flag |= (quint64(1) << i);
            } else {
                if (containsCurrent) {
                    flag |= (quint64(1) << i);
                }
            }
        }
    }
    return flag;
}

quint64 CheckBoxGroup::checkedFlags(int maxChecked) const
{
    int count = 0;
    quint64 flag = 0;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            flag |= (quint64(1) << i);
            count++;
            if (count >= maxChecked) {
                break;
            }
        }
    }
    return flag;
}

void CheckBoxGroup::setCheckBoxTest(QStringList &textList)
{
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        checkBox->setText(textList[i]);
    }
}

void CheckBoxGroup::clearCheckBoxList()
{
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        ui->gridLayout->removeWidget(checkBox);
        delete checkBox;
    }
    m_checkBoxList.clear();
}

void CheckBoxGroup::onLanguageChanged()
{
    ui->checkBoxAll->setText(GET_TEXT("SYSTEMGENERAL/164000", "All"));
}

void CheckBoxGroup::onCheckBoxClicked()
{
    int checkedCount = 0;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            checkedCount++;
        }
    }
    if (checkedCount == 0) {
        ui->checkBoxAll->setCheckState(Qt::Unchecked);
    } else if (checkedCount >= m_checkBoxList.size()) {
        ui->checkBoxAll->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxAll->setCheckState(Qt::PartiallyChecked);
    }

    //
    emit checkBoxClicked();
}

void CheckBoxGroup::on_checkBoxAll_clicked(bool checked)
{
    if (checked) {
        ui->checkBoxAll->setCheckState(Qt::Checked);
    }

    //
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isEnabled()) {
            checkBox->setChecked(checked);
        }
    }

    //
    emit checkBoxClicked();
}
