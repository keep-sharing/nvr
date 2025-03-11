#include "pathwidget.h"
#include "ui_pathwidget.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "msuser.h"
#include "patternwidget.h"
#include "ptzcontrol.h"
#include "ptzdatamanager.h"
#include "MsDevice.h"

#ifdef MS_FISHEYE_SOFT_DEWARP
#else
#include "FisheyeControl.h"
#endif

PathWidget::PathWidget(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::PathWidget)
{
    ui->setupUi(this);

    m_itemDelegate = new PatrolItemDelegate(this);
    connect(m_itemDelegate, SIGNAL(buttonClicked(int, int)), this, SLOT(itemButtonClicked(int, int)));
    ui->treeWidget->setItemDelegate(m_itemDelegate);
    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    //巡航 编辑
    QStringList tableLabels;
    tableLabels << "No.";
    tableLabels << "Preset";
    tableLabels << "Speed";
    tableLabels << "Time(s)";
    ui->tableWidget_setting->setColumnCount(tableLabels.size());
    ui->tableWidget_setting->setHorizontalHeaderLabels(tableLabels);

    m_patrolEditDelegate = new PatrolEditDelegate(ui->tableWidget_setting);
    ui->tableWidget_setting->setItemDelegate(m_patrolEditDelegate);
    ui->tableWidget_setting->setColumnWidth(0, 40);
    ui->tableWidget_setting->setColumnWidth(1, 70);
    ui->tableWidget_setting->setColumnWidth(2, 70);
}

PathWidget::~PathWidget()
{
    delete ui;
}

void PathWidget::initializeData(int channel, const resp_ptz_tour *tour_array, int count)
{
    m_channel = channel;
    m_patrolEditDelegate->setChannel(m_channel);
    ui->stackedWidget->setCurrentIndex(0);

    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(3);
    ui->treeWidget->setColumnWidth(0, 40);
    ui->treeWidget->setColumnWidth(1, 100);

    m_tourList.clear();
    for (int i = 0; i < count; ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, QString("%1").arg(i + 1, 2, 10, QLatin1Char('0')));
        item->setText(1, QString("Path %1").arg(i + 1));
        ui->treeWidget->addTopLevelItem(item);
        //
        const resp_ptz_tour &tour = tour_array[i];
        m_tourList.append(tour);
        if (tour.preset_id[0] != 0) {
            ui->treeWidget->setPatrolState(i, PatrolItemDelegate::ItemEnable);
        } else {
            ui->treeWidget->setPatrolState(i, PatrolItemDelegate::ItemDisable);
        }
    }
}

void PathWidget::clear()
{
    ui->treeWidget->clear();
    ui->stackedWidget->setCurrentIndex(0);
}

void PathWidget::clearPlayingState()
{
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        PatrolItemDelegate::ItemType type = ui->treeWidget->patrolState(i);
        if (type == PatrolItemDelegate::ItemPlaying) {
            ui->treeWidget->setPatrolState(i, PatrolItemDelegate::ItemEnable, true);
        }
    }
}

/**
 * @brief PathWidget::sendPatrolControl
 * @param action: REQUEST_FLAG_PTZ_TOUR_RUN, REQUEST_FLAG_PTZ_TOUR_STOP, REQUEST_FLAG_PTZ_TOUR_CLEAR
 * @param tourid: 1-8
 */
void PathWidget::sendPatrolControl(int action, int tourid)
{
    m_channel = PtzControl::instance()->currentChannel();
    struct req_ptz_tour ptz_tour;
    memset(&ptz_tour, 0, sizeof(req_ptz_tour));
    ptz_tour.chn = m_channel;
    ptz_tour.tourid = tourid;
    if (gPtzDataManager->isFisheye()) {
        ptz_tour.type = 1;
        ptz_tour.stream_id = gPtzDataManager->fisheyeStream();
    }

    sendMessageOnly(action, (void *)&ptz_tour, sizeof(req_ptz_tour));
}

/**
 * @brief PathWidget::sendPatrolData
 * @param tourid: 1-8
 */
void PathWidget::sendPatrolData(int tourid)
{
    struct req_ptz_tour_all ptz_tour_all;
    memset(&ptz_tour_all, 0, sizeof(req_ptz_tour_all));
    ptz_tour_all.chn = m_channel;
    ptz_tour_all.tourid = tourid;
    ptz_tour_all.rowsize = ui->tableWidget_setting->rowCount();
    for (int row = 0; row < ptz_tour_all.rowsize; ++row) {
        int preset = ui->tableWidget_setting->item(row, 1)->text().toInt();
        int speed = ui->tableWidget_setting->item(row, 2)->text().toInt();
        int time = ui->tableWidget_setting->item(row, 3)->text().toInt();

        ptz_path &path = ptz_tour_all.path[row];
        path.preset_id = preset;
        path.timeout = time;
        path.speed = speed;
    }
    sendMessage(REQUEST_FLAG_PTZ_SET, &ptz_tour_all, sizeof(req_ptz_tour_all));
}

void PathWidget::sendFisheyePatrolData(int tourid)
{
    struct fisheye_ptz_tour ptz_tour;
    memset(&ptz_tour, 0, sizeof(fisheye_ptz_tour));
    ptz_tour.chn = m_channel;
    ptz_tour.param = tourid - 1;
    ptz_tour.stream_id = gPtzDataManager->fisheyeStream();
    resp_ptz_tour &tour = ptz_tour.tour[tourid - 1];
    for (int row = 0; row < MS_KEY_MAX && row < ui->tableWidget_setting->rowCount(); ++row) {
        int preset = ui->tableWidget_setting->item(row, 1)->text().toInt();
        int speed = ui->tableWidget_setting->item(row, 2)->text().toInt();
        int time = ui->tableWidget_setting->item(row, 3)->text().toInt();

        tour.preset_id[row] = preset;
        tour.timeout[row] = time;
        tour.speed[row] = speed;
    }
    sendMessageOnly(REQUEST_FLAG_PTZ_FISH_TOUR, &ptz_tour, sizeof(fisheye_ptz_tour));
}

void PathWidget::updatePatrolDataFromTable()
{
    resp_ptz_tour &tour = m_tourList[m_currentPathIndex];
    memset(&tour, 0, sizeof(resp_ptz_tour));
    for (int row = 0; row < ui->tableWidget_setting->rowCount(); ++row) {
        int preset = ui->tableWidget_setting->item(row, 1)->text().toInt();
        int speed = ui->tableWidget_setting->item(row, 2)->text().toInt();
        int time = ui->tableWidget_setting->item(row, 3)->text().toInt();
        tour.preset_id[row] = preset;
        tour.speed[row] = speed;
        tour.timeout[row] = time;
    }

    if (tour.preset_id[0] != 0) {
        ui->treeWidget->setPatrolState(m_currentPathIndex, PatrolItemDelegate::ItemEnable, true);
    } else {
        ui->treeWidget->setPatrolState(m_currentPathIndex, PatrolItemDelegate::ItemDisable, true);
    }
}

void PathWidget::updateItemButton(int row, int column)
{
    QTreeWidgetItem *item = ui->treeWidget->topLevelItem(row);
    if (item) {
        ui->treeWidget->closePersistentEditor(item, column);
        ui->treeWidget->openPersistentEditor(item, column);
    }
}

void PathWidget::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    int currentRow = ui->treeWidget->indexOfTopLevelItem(current);
    int previousRow = ui->treeWidget->indexOfTopLevelItem(previous);
    if (currentRow != previousRow) {
        ui->treeWidget->openPersistentEditor(current, 2);
        ui->treeWidget->closePersistentEditor(previous, 2);
    }
}

void PathWidget::itemButtonClicked(int row, int index)
{
    m_currentPathIndex = row;
    int tourid = row + 1;
    switch (index) {
    case 0: //play stop
    {
        const PatrolItemDelegate::ItemType &type = ui->treeWidget->patrolState(row);
        switch (type) {
        case PatrolItemDelegate::ItemEnable:
            sendPatrolControl(REQUEST_FLAG_PTZ_TOUR_RUN, tourid);
            ui->treeWidget->setPatrolState(row, PatrolItemDelegate::ItemPlaying, true);
            PtzControl::instance()->patternWidget()->clearPlayingState();
            break;
        case PatrolItemDelegate::ItemPlaying:
            sendPatrolControl(REQUEST_FLAG_PTZ_TOUR_STOP, tourid);
            ui->treeWidget->setPatrolState(row, PatrolItemDelegate::ItemEnable, true);
            break;
        default:
            break;
        }
        break;
    }
    case 1: //setting
    {
        ui->stackedWidget->setCurrentIndex(1);
        m_currentPathIndex = row;
        ui->label_pathName->setText(QString("Path%1").arg(m_currentPathIndex + 1));
        const resp_ptz_tour &tour = m_tourList.at(row);
        ui->tableWidget_setting->setRowCount(0);
        int row = 0;
        for (int i = 0; i < MS_KEY_MAX; ++i) {
            if (tour.preset_id[i] == 0) {
                break;
            }
            ui->tableWidget_setting->insertRow(row);
            QTableWidgetItem *item0 = new QTableWidgetItem(QString("%1").arg(i + 1, 2, 10, QLatin1Char('0')));
            ui->tableWidget_setting->setItem(i, 0, item0);
            QTableWidgetItem *item1 = new QTableWidgetItem(QString("%1").arg(tour.preset_id[i]));
            ui->tableWidget_setting->setItem(i, 1, item1);
            QTableWidgetItem *item2 = new QTableWidgetItem(QString("%1").arg(tour.speed[i]));
            ui->tableWidget_setting->setItem(i, 2, item2);
            QTableWidgetItem *item3 = new QTableWidgetItem(QString("%1").arg(tour.timeout[i]));
            ui->tableWidget_setting->setItem(i, 3, item3);
            row++;
        }
        break;
    }
    case 2: //delete
    {
        updatePatrolDataFromTable();
        sendPatrolControl(REQUEST_FLAG_PTZ_TOUR_CLEAR, tourid);
        ui->treeWidget->setPatrolState(row, PatrolItemDelegate::ItemDisable, true);
        break;
    }
    default:
        break;
    }
}

void PathWidget::on_pushButton_add_clicked()
{
    int row = ui->tableWidget_setting->rowCount();
    if (row >= 48) {
        return;
    }
    ui->tableWidget_setting->insertRow(row);
    QTableWidgetItem *item0 = new QTableWidgetItem(QString("%1").arg(row + 1, 2, 10, QLatin1Char('0')));
    ui->tableWidget_setting->setItem(row, 0, item0);
    int preset = row + 1;
    if (!qMsNvr->isFisheye(m_channel)) {
        if (preset > 32) {
            preset = row - 32 + 53;
        }
    }
    QTableWidgetItem *item1 = new QTableWidgetItem(QString("%1").arg(preset));
    ui->tableWidget_setting->setItem(row, 1, item1);
    QTableWidgetItem *item2 = new QTableWidgetItem("30");
    ui->tableWidget_setting->setItem(row, 2, item2);
    QTableWidgetItem *item3 = new QTableWidgetItem("15");
    if (gPtzDataManager->isPtzBullet()) {
        item3->setText("15");
    } else {
        item3->setText("10");
    }
    ui->tableWidget_setting->setItem(row, 3, item3);
    ui->tableWidget_setting->scrollToBottom();
}

void PathWidget::on_pushButton_delete_clicked()
{
    int currentRow = ui->tableWidget_setting->currentRow();
    //
    ui->tableWidget_setting->removeRow(currentRow);
    int rowCount = ui->tableWidget_setting->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        ui->tableWidget_setting->item(i, 0)->setText(QString("%1").arg(i + 1, 2, 10, QLatin1Char('0')));
    }
    if (currentRow < rowCount) {
        ui->tableWidget_setting->selectRow(currentRow);
    } else {
        ui->tableWidget_setting->selectRow(currentRow - 1);
    }
}

void PathWidget::on_pushButton_up_clicked()
{
    int currentRow = ui->tableWidget_setting->currentRow();
    int destRow = currentRow - 1;
    if (destRow < 0) {
        return;
    }
    QString temp1 = ui->tableWidget_setting->item(destRow, 1)->text();
    QString temp2 = ui->tableWidget_setting->item(destRow, 2)->text();
    QString temp3 = ui->tableWidget_setting->item(destRow, 3)->text();
    ui->tableWidget_setting->item(destRow, 1)->setText(ui->tableWidget_setting->item(currentRow, 1)->text());
    ui->tableWidget_setting->item(destRow, 2)->setText(ui->tableWidget_setting->item(currentRow, 2)->text());
    ui->tableWidget_setting->item(destRow, 3)->setText(ui->tableWidget_setting->item(currentRow, 3)->text());
    ui->tableWidget_setting->item(currentRow, 1)->setText(temp1);
    ui->tableWidget_setting->item(currentRow, 2)->setText(temp2);
    ui->tableWidget_setting->item(currentRow, 3)->setText(temp3);
    ui->tableWidget_setting->selectRow(destRow);
}

void PathWidget::on_pushButton_down_clicked()
{
    int currentRow = ui->tableWidget_setting->currentRow();
    if (currentRow < 0) {
        return;
    }

    int destRow = currentRow + 1;
    if (destRow >= ui->tableWidget_setting->rowCount()) {
        return;
    }
    QString temp1 = ui->tableWidget_setting->item(destRow, 1)->text();
    QString temp2 = ui->tableWidget_setting->item(destRow, 2)->text();
    QString temp3 = ui->tableWidget_setting->item(destRow, 3)->text();
    ui->tableWidget_setting->item(destRow, 1)->setText(ui->tableWidget_setting->item(currentRow, 1)->text());
    ui->tableWidget_setting->item(destRow, 2)->setText(ui->tableWidget_setting->item(currentRow, 2)->text());
    ui->tableWidget_setting->item(destRow, 3)->setText(ui->tableWidget_setting->item(currentRow, 3)->text());
    ui->tableWidget_setting->item(currentRow, 1)->setText(temp1);
    ui->tableWidget_setting->item(currentRow, 2)->setText(temp2);
    ui->tableWidget_setting->item(currentRow, 3)->setText(temp3);
    ui->tableWidget_setting->selectRow(destRow);
}

void PathWidget::on_pushButton_save_clicked()
{
    int tourid = m_currentPathIndex + 1;
    updatePatrolDataFromTable();

    if (gPtzDataManager->isFisheye()) {
        sendFisheyePatrolData(tourid);
    } else {
        sendPatrolData(tourid);
    }

    ui->stackedWidget->setCurrentIndex(0);
}

void PathWidget::on_pushButton_cancel_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}
