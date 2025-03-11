#include "PTZLimitsPanel.h"
#include "ui_PTZLimitsPanel.h"

#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include "msuser.h"
#include "ptzdatamanager.h"
#include <QtDebug>

extern "C" {
#include "ptz_public.h"
}

PTZLimitsPanel::PTZLimitsPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PTZLimitsPanel)
{
    ui->setupUi(this);
    QFont font = ui->pushButtonClearAll->font();
    font.setPixelSize(13);
    ui->pushButtonClearAll->setFont(font);
    ui->pushButtonClearAll->setStyleSheet("padding-left: 5px;padding-right: 5px;min-width: 60px;min-height: 18px;background-color: #0aa8e3;");

    m_itemDelegate = new PresetItemDelegate(this);
    m_itemDelegate->setTheme(PresetItemDelegate::GrayTheme);
    connect(m_itemDelegate, SIGNAL(buttonClicked(int, int)), this, SLOT(itemButtonClicked(int, int)));
    ui->treeWidget->setItemDelegate(m_itemDelegate);

    onLanguageChanged();
}

PTZLimitsPanel::~PTZLimitsPanel()
{
    delete ui;
}

void PTZLimitsPanel::initializeData(const int mask, const int mode, const int supportSpeed)
{
    m_mode = mode;
    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(3);
    ui->treeWidget->setColumnWidth(0, 220);
    ui->treeWidget->hideColumn(1);
    ui->treeWidget->setAlternatingRowColors(true);
    QStringList name;
    m_count = supportSpeed ? 4 : 2;
    name.append("    " + GET_TEXT("PTZCONFIG/166008", "Left Limit"));
    name.append("    " + GET_TEXT("PTZCONFIG/166009", "Right Limit"));
    name.append("    " + GET_TEXT("PTZCONFIG/166010", "Up Limit"));
    name.append("    " + GET_TEXT("PTZCONFIG/166011", "Down Limit"));

    for (int i = 0; i < m_count; ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(0, name[i]);
        if (i % 2 == 0) {
            item->setBackgroundColor(0, QColor("#E1E1E1"));
            item->setBackgroundColor(2, QColor("#E1E1E1"));
        }
        ui->treeWidget->addTopLevelItem(item);

        if (mask & (1 << i)) {
            ui->treeWidget->setPresetState(i, PresetItemDelegate::ItemLimits);
        } else {
            ui->treeWidget->setPresetState(i, PresetItemDelegate::ItemLimitsDisable);
        }
        ui->treeWidget->openPersistentEditor(item, 2);
    }
}

void PTZLimitsPanel::onLanguageChanged()
{
    ui->label->setText(GET_TEXT("PTZCONFIG/166006", "Limit"));
    ui->pushButtonClearAll->setText(GET_TEXT("PTZCONFIG/166007", "Clear All"));
}

void PTZLimitsPanel::itemButtonClicked(int row, int index)
{
    switch (index) {
    case 1: //save
        ui->treeWidget->setPresetState(row, PresetItemDelegate::ItemLimits, true);
        break;
    case 2: //delete
        ui->treeWidget->setPresetState(row, PresetItemDelegate::ItemLimitsDisable, true);
        break;
    }
    PTZ_LIMITS_TYPE type;
    switch (row) {
    case 0:
        type = m_mode ? PTZ_LIMITS_SCAN_SET_LEFT : PTZ_LIMITS_MANUAL_SET_LEFT;
        break;
    case 1:
        type = m_mode ? PTZ_LIMITS_SCAN_SET_RIGHT : PTZ_LIMITS_MANUAL_SET_RIGHT;
        break;
    case 2:
        type = m_mode ? PTZ_LIMITS_SCAN_SET_UP : PTZ_LIMITS_MANUAL_SET_UP;
        break;
    case 3:
        type = m_mode ? PTZ_LIMITS_SCAN_SET_DOWN : PTZ_LIMITS_MANUAL_SET_DOWN;
        break;
    default:
        type = PTZ_LIMITS_NONE;
        break;
    }
    emit setLimitControl(type, (index == 1) ? 1 : 0);
}

void PTZLimitsPanel::on_pushButtonClearAll_clicked()
{
    for (int i = 0; i < m_count; ++i) {
        ui->treeWidget->setPresetState(i, PresetItemDelegate::ItemLimitsDisable, true);
    }
    if (m_mode == 0) {
        emit setLimitControl(PTZ_LIMITS_MANUAL_CLEAR_ALL, 1);
    } else {
        emit setLimitControl(PTZ_LIMITS_SCAN_CLEAR_ALL, 1);
    }
}
