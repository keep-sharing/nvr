#include "presetwidget.h"
#include "ui_presetwidget.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "msuser.h"
#include "ptzcontrol.h"
#include "ptzdatamanager.h"
#include <QtDebug>

#ifdef MS_FISHEYE_SOFT_DEWARP
#else
#include "FisheyeControl.h"
#endif

extern "C" {
#include "ptz_public.h"
}

PresetWidget::PresetWidget(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::PresetWidget)
{
    ui->setupUi(this);

    m_itemDelegate = new PresetItemDelegate(this);
    connect(m_itemDelegate, SIGNAL(buttonClicked(int, int)), this, SLOT(itemButtonClicked(int, int)));
    ui->treeWidget->setItemDelegate(m_itemDelegate);
    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
}

PresetWidget::~PresetWidget()
{
    delete ui;
}

void PresetWidget::initializeData(const resp_ptz_preset *preset_array, int count)
{
    m_itemDelegate->setEditable(gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZSETTINGS));

    ui->treeWidget->clear();
    ui->treeWidget->clearSelection();
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
            if (PtzControl::instance()->isFisheye()) {
                if (preset.enable) {
                    ui->treeWidget->setPresetState(i, PresetItemDelegate::ItemNormal);
                } else {
                    ui->treeWidget->setPresetState(i, PresetItemDelegate::ItemDisable);
                }
            } else {
                ui->treeWidget->setPresetState(i, PresetItemDelegate::ItemValidDefault);
            }
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

void PresetWidget::clear()
{
    ui->treeWidget->clear();
}

void PresetWidget::setFisheyePresetName(int index, const QString &name)
{
    struct req_http_common_param req;
    memset(&req, 0, sizeof(struct req_http_common_param));

    req.chnid = PtzControl::instance()->currentChannel();
    snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting.html");
    snprintf(req.info.key, sizeof(req.info.key), "%s", "ipncptz=setfishpreset");
    snprintf(req.info.value, sizeof(req.info.value), "%d:%d:%s", gPtzDataManager->fisheyeStream(), index + 1, name.toStdString().c_str());
    sendMessage(REQUEST_FLAG_SET_IPC_COMMON_PARAM, (void *)&req, sizeof(struct req_http_common_param));
}

void PresetWidget::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    int currentRow = ui->treeWidget->indexOfTopLevelItem(current);
    int previousRow = ui->treeWidget->indexOfTopLevelItem(previous);
    if (currentRow != previousRow) {
        ui->treeWidget->openPersistentEditor(current, 2);
        ui->treeWidget->closePersistentEditor(previous, 2);
    }
}

void PresetWidget::itemButtonClicked(int row, int index)
{
    QString name = ui->treeWidget->presetName(row);
    switch (index) {
    case 0: //play
        if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL)) {
            ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
            return;
        }
        if (gPtzDataManager->isSupportWiper() && row == 52) {
          gPtzDataManager->sendPresetControlJson(IPC_PTZ_CONTORL_WIPER, 1);
        } else {
          sendPresetControl(PTZ_PRESET_GOTO, row, name);
        }
        break;
    case 1: //save
        if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZSETTINGS)) {
            ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
            return;
        }
        sendPresetControl(PTZ_PRESET_SET, row, name);
        if (gPtzDataManager->isFisheye()) {
            setFisheyePresetName(row, name);
        }
        ui->treeWidget->setPresetState(row, PresetItemDelegate::ItemNormal, true);
        break;
    case 2: //delete
        if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZSETTINGS)) {
            ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
            return;
        }
        sendPresetControl(PTZ_PRESET_CLEAR, row, name);
        ui->treeWidget->setPresetState(row, PresetItemDelegate::ItemDisable, true);
        break;
    }
}
