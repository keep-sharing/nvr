#include "patternwidget.h"
#include "ui_patternwidget.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "msuser.h"
#include "ptzcontrol.h"
#include "pathwidget.h"

extern "C" {
#include "ptz_public.h"
}

PatternWidget::PatternWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PatternWidget)
{
    ui->setupUi(this);

    m_itemDelegate = new PatternItemDelegate(this);
    connect(m_itemDelegate, SIGNAL(buttonClicked(int, int)), this, SLOT(itemButtonClicked(int, int)));
    ui->treeWidget->setItemDelegate(m_itemDelegate);
    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
}

PatternWidget::~PatternWidget()
{
    delete ui;
}

void PatternWidget::initializeData(const int *pattern_array, int count)
{
    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(3);
    ui->treeWidget->setColumnWidth(0, 40);
    ui->treeWidget->setColumnWidth(1, 100);
    //
    for (int i = 0; i < count; ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, QString("%1").arg(i + 1, 2, 10, QLatin1Char('0')));
        item->setText(1, QString("Pattern %1").arg(i + 1));
        ui->treeWidget->addTopLevelItem(item);
        //
        const int &enable = pattern_array[i];
        if (enable) {
            ui->treeWidget->setPatternState(i, PatternItemDelegate::ItemEnable);
        } else {
            ui->treeWidget->setPatternState(i, PatternItemDelegate::ItemDisable);
        }
    }
}

void PatternWidget::clear()
{
    ui->treeWidget->clear();
}

void PatternWidget::clearPlayingState()
{
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        PatternItemDelegate::ItemType type = ui->treeWidget->patternState(i);
        switch (type) {
        case PatternItemDelegate::ItemPlaying:
        case PatternItemDelegate::ItemRecording:
            ui->treeWidget->setPatternState(i, PatternItemDelegate::ItemEnable, true);
            break;
        default:
            break;
        }
    }
}

void PatternWidget::updateItemButton(int row, int column)
{
    QTreeWidgetItem *item = ui->treeWidget->topLevelItem(row);
    if (item) {
        ui->treeWidget->closePersistentEditor(item, column);
        ui->treeWidget->openPersistentEditor(item, column);
    }
}

void PatternWidget::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    int currentRow = ui->treeWidget->indexOfTopLevelItem(current);
    int previousRow = ui->treeWidget->indexOfTopLevelItem(previous);
    if (currentRow != previousRow) {
        ui->treeWidget->openPersistentEditor(current, 2);
        ui->treeWidget->closePersistentEditor(previous, 2);
    }
}

void PatternWidget::itemButtonClicked(int row, int index)
{
    qDebug() << QString("PatternWidget::itemButtonClicked, row: %1, index: %2").arg(row).arg(index);

    switch (index) {
    case 0: //play stop
    {
        const PatternItemDelegate::ItemType &type = ui->treeWidget->patternState(row);
        switch (type) {
        case PatternItemDelegate::ItemEnable:
            emit sendPatternControl(PTZ_PATTERN_RUN, row);
            ui->treeWidget->setPatternState(row, PatternItemDelegate::ItemPlaying, true);
            PtzControl::instance()->pathWidget()->clearPlayingState();
            break;
        case PatternItemDelegate::ItemPlaying:
            emit sendPatternControl(PTZ_STOP_ALL, row);
            ui->treeWidget->setPatternState(row, PatternItemDelegate::ItemEnable, true);
            break;
        default:
            break;
        }
        break;
    }
    case 1: //record
    {
        const PatternItemDelegate::ItemType &type = ui->treeWidget->patternState(row);
        switch (type) {
        case PatternItemDelegate::ItemDisable:
        case PatternItemDelegate::ItemEnable:
            emit sendPatternControl(PTZ_PATTERN_START, row);
            ui->treeWidget->setPatternState(row, PatternItemDelegate::ItemRecording, true);
            PtzControl::instance()->pathWidget()->clearPlayingState();
            break;
        case PatternItemDelegate::ItemRecording:
            emit sendPatternControl(PTZ_PATTERN_STOP, row);
            ui->treeWidget->setPatternState(row, PatternItemDelegate::ItemEnable, true);
            break;
        default:
            break;
        }
        break;
    }
    case 2: //delete
    {
        emit sendPatternControl(PTZ_PATTERN_DEL, row);
        ui->treeWidget->setPatternState(row, PatternItemDelegate::ItemDisable, true);
        break;
    }
    default:
        break;
    }
}
