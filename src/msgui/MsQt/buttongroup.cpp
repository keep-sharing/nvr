#include "buttongroup.h"
#include "ui_buttongroup.h"
#include <qmath.h>
#include "MsDevice.h"

ButtonGroup::ButtonGroup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ButtonGroup)
{
    ui->setupUi(this);

    m_buttonGroup = new QButtonGroup(this);
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonClicked(int)));
}

ButtonGroup::~ButtonGroup()
{
    delete ui;
}

void ButtonGroup::setCount(int count)
{
    m_count = count;

    if (m_count < 8) {
        ui->widget_right->show();
    } else {
        ui->widget_right->hide();
    }

    int row = 0;
    int column = 0;
    for (int i = 0; i < count; ++i) {
        MyPushButton *button = new MyPushButton(this);
        button->setCheckable(true);
        button->setText(QString::number(i + 1));
        m_buttonGroup->addButton(button, i);

        row = i / m_columnCount;
        column = i % m_columnCount;
        if (column >= m_columnCount) {
            column = 0;
        }
        ui->gridLayout->addWidget(button, row, column);
    }
}

void ButtonGroup::setCountFromChannelName(int count)
{
    m_count = count;

    if (m_count < 8) {
        ui->widget_right->show();
    } else {
        ui->widget_right->hide();
    }

    int row = 0;
    int column = 0;
    for (int i = 0; i < count; ++i) {
        MyPushButton *button = new MyPushButton(this);
        button->setCheckable(true);
        QString channelName = qMsNvr->channelName(i);
        QFontMetrics fontWidth(this->font());
        QString elidnote = fontWidth.elidedText(channelName, Qt::ElideRight, 78);
        button->setText(elidnote);
        m_buttonGroup->addButton(button, i);

        row = i / m_columnCount;
        column = i % m_columnCount;
        if (column >= m_columnCount) {
            column = 0;
        }
        ui->gridLayout->addWidget(button, row, column);
    }
}

int ButtonGroup::count()
{
    return m_count;
}

void ButtonGroup::setCurrentIndex(int index)
{
    m_buttonGroup->button(index)->setChecked(true);
    onButtonClicked(index);
}

int ButtonGroup::currentIndex() const
{
    return m_buttonGroup->checkedId();
}

void ButtonGroup::editCurrentIndex(int index)
{
    m_buttonGroup->button(index)->setChecked(true);
}

void ButtonGroup::onButtonClicked(int index)
{
    emit buttonClicked(index);
}
