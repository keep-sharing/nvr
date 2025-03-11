#include "MyUserCheckBoxGroup.h"
#include "ui_MyUserCheckBoxGroup.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "qmath.h"
#include <QPainter>

MyUserCheckBoxGroup::MyUserCheckBoxGroup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyUserCheckBoxGroup)
{
    ui->setupUi(this);

    onLanguageChanged();
}

MyUserCheckBoxGroup::~MyUserCheckBoxGroup()
{
    delete ui;
}

void MyUserCheckBoxGroup::setCount(int allCount, int columnCount)
{
    m_columnCount = columnCount;

    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        ui->gridLayoutCheckBox->removeWidget(checkBox);
        delete checkBox;
    }
    m_checkBoxList.clear();

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
                ui->gridLayoutCheckBox->addWidget(checkBox, r, c);
                m_checkBoxList.append(checkBox);
                channel++;
            } else {
                break;
            }
        }
    }
}

void MyUserCheckBoxGroup::setCheckBoxStyle()
{
    this->setStyleSheet("background: rgb(225, 225, 225); border: 1px solid rgb(185, 185, 185);");
    ui->checkBoxAll->setStyleSheet("min-height: 27px;border: 0px");
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        checkBox->setStyleSheet("min-height: 27px;border: 0px");
    }
    update();
}

void MyUserCheckBoxGroup::setCheckBoxUserStyle()
{
    this->setStyleSheet("background: rgb(190, 190, 190); border: 1px solid rgb(140, 140, 140);");
    ui->checkBoxAll->setStyleSheet("min-height: 27px;border: 0px");
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        checkBox->setStyleSheet("min-height: 27px;border: 0px");
    }
    update();
}

void MyUserCheckBoxGroup::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MyUserCheckBoxGroup::clearCheck()
{
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        checkBox->setChecked(false);
    }
    ui->checkBoxAll->setChecked(false);
}

bool MyUserCheckBoxGroup::hasChannelSelected()
{
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isVisible() && checkBox->isEnabled() && checkBox->isChecked()) {
            return true;
        }
    }
    return false;
}

void MyUserCheckBoxGroup::setCheckedFromString(const QString &text)
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

void MyUserCheckBoxGroup::setCheckedFromInt(const quint32 value)
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

void MyUserCheckBoxGroup::setAllChecked()
{
    ui->checkBoxAll->setChecked(true);
    on_checkBoxAll_clicked(true);
}

QList<bool> MyUserCheckBoxGroup::checkStateList()
{
    QList<bool> list;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        list.append(checkBox->isChecked());
    }
    return list;
}

void MyUserCheckBoxGroup::setIndexEnabled(int index, bool enable)
{
    if (m_checkBoxList.size() > index) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(index);
        checkBox->setChecked(true);
        checkBox->setEnabled(enable);
    }
}

QList<int> MyUserCheckBoxGroup::checkedList(bool containsCurrent /* = true*/) const
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

QList<int> MyUserCheckBoxGroup::checkedList(int maxChecked) const
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

QString MyUserCheckBoxGroup::checkedMask() const
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

QString MyUserCheckBoxGroup::checkedMask(int maxChecked) const
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

quint64 MyUserCheckBoxGroup::checkedFlags(bool containsCurrent /* = true*/) const
{
    quint64 flag = 0;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            if (checkBox->isEnabled()) {
                flag |= (static_cast<quint64>(1) << i);
            } else {
                if (containsCurrent) {
                    flag |= (static_cast<quint64>(1) << i);
                }
            }
        }
    }
    return flag;
}

quint64 MyUserCheckBoxGroup::checkedFlags(int maxChecked) const
{
    int count = 0;
    quint64 flag = 0;
    for (int i = 0; i < m_checkBoxList.size(); ++i) {
        ChannelCheckBox *checkBox = m_checkBoxList.at(i);
        if (checkBox->isChecked()) {
            flag |= (static_cast<quint64>(1) << i);
            count++;
            if (count >= maxChecked) {
                break;
            }
        }
    }
    return flag;
}

void MyUserCheckBoxGroup::setAllButtonText(QString text)
{
    ui->checkBoxAll->setText(text);
}

void MyUserCheckBoxGroup::onLanguageChanged()
{
    ui->checkBoxAll->setText(GET_TEXT("SYSTEMGENERAL/164000", "All"));
}

void MyUserCheckBoxGroup::onCheckBoxClicked()
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

void MyUserCheckBoxGroup::on_checkBoxAll_clicked(bool checked)
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
