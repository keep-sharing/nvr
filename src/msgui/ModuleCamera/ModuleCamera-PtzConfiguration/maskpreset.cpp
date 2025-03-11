#include "maskpreset.h"
#include "ui_maskpreset.h"

#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "msuser.h"
#include "ptzdatamanager.h"

#include <QtDebug>

extern "C" {
#include "ptz_public.h"
}

MaskPreset::MaskPreset(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MaskPreset)
{
    ui->setupUi(this);

    m_itemDelegate = new PresetItemDelegate(this);
    m_itemDelegate->setTheme(PresetItemDelegate::GrayTheme);
    connect(m_itemDelegate, SIGNAL(buttonClicked(int, int)), this, SLOT(itemButtonClicked(int, int)));
    ui->treeWidget->setItemDelegate(m_itemDelegate);
    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
}

MaskPreset::~MaskPreset()
{
    delete ui;
}

void MaskPreset::initializeData(const resp_ptz_preset *preset_array, int count)
{
    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(3);
    ui->treeWidget->setColumnWidth(0, 40);
    ui->treeWidget->setColumnWidth(1, 100);

    for (int i = 0; i < count; ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(0, QString("%1").arg(i + 1, 3, 10, QLatin1Char('0')));
        ui->treeWidget->addTopLevelItem(item);
        //
        const resp_ptz_preset &preset = preset_array[i];
        if (QString(preset.name).isEmpty()) {
            ui->treeWidget->setPresetName(i, QString("Preset %1").arg(i + 1));
        } else {
            ui->treeWidget->setPresetName(i, preset.name);
        }

        /**以下是特殊处理，如有修改，需要备注清楚**/
        //雨刷机型第53个点也为特殊预置点
        int limit = 51;
        gPtzDataManager->waitForGetSystemInfo();
        if (gPtzDataManager->isSupportWiper()) {
            limit = 52;
            ui->treeWidget->setPresetName(limit, "Mauanl Wiper");
        }

        //固定值
        if (i >= 32 && i <= limit) {
            ui->treeWidget->setPresetState(i, PresetItemDelegate::ItemValidDefault);
        } else {
            if (preset.enable) {
                ui->treeWidget->setPresetState(i, PresetItemDelegate::ItemNormal);
            } else {
                ui->treeWidget->setPresetState(i, PresetItemDelegate::ItemDisable);
            }
        }

        //MSHN-5829 MS-Cxx61机型的PTZ控制面板中，预置点中auto flip；tilt scan；panorama scan应置灰(无效)
        switch (i) {
        case 32:
        case 50:
        case 51:
            if (gPtzDataManager->isPtzBullet()) {
                ui->treeWidget->setPresetState(i, PresetItemDelegate::ItemInvalidDefault);
            }
            break;
        }
    }
}

void MaskPreset::clear()
{
    ui->treeWidget->clear();
}

void MaskPreset::onLanguageChanged()
{
    ui->label->setText(GET_TEXT("PTZDIALOG/21015", "Preset"));
}

void MaskPreset::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    int currentRow = ui->treeWidget->indexOfTopLevelItem(current);
    int previousRow = ui->treeWidget->indexOfTopLevelItem(previous);
    if (currentRow != previousRow) {
        ui->treeWidget->openPersistentEditor(current, 2);
        ui->treeWidget->closePersistentEditor(previous, 2);
    }
}

void MaskPreset::itemButtonClicked(int row, int index)
{
    int id = row + 1;
    int speed = gPtzDataManager->ptzSpeed();
    QString name = ui->treeWidget->presetName(row);
    switch (index) {
    case 0: //play
        if (gPtzDataManager->isSupportWiper() && id == 53) {
            gPtzDataManager->sendPresetControlJson(IPC_PTZ_CONTORL_WIPER, 1);
        } else {
            gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, id, speed, name);
        }
        break;
    case 1: //save
        gPtzDataManager->sendPresetControl(PTZ_PRESET_SET, id, speed, name);
        ui->treeWidget->setPresetState(row, PresetItemDelegate::ItemNormal, true);
        break;
    case 2: //delete
        gPtzDataManager->sendPresetControl(PTZ_PRESET_CLEAR, id, speed, name);
        ui->treeWidget->setPresetState(row, PresetItemDelegate::ItemDisable, true);
        break;
    }
}
