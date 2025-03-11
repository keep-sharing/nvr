#include "CustomLayoutDialog.h"
#include "ui_CustomLayoutDialog.h"
#include "CustomLayoutCommand.h"
#include "CustomLayoutData.h"
#include "CustomLayoutScene.h"
#include "ItemDelegateCustomLayout.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MultiScreenControl.h"
#include "MyDebug.h"
#include <QUndoStack>

CustomLayoutDialog::CustomLayoutDialog(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::CustomLayoutDialog)
{
    ui->setupUi(this);

    m_scene = new CustomLayoutScene(this);
    ui->graphicsView->setScene(m_scene);

    ui->comboBoxBasic->clear();
    int maxCount = qMsNvr->maxChannel() == 64 ? 8 : 5;
    for (int i = 0; i < maxCount; ++i) {
        ui->comboBoxBasic->addItem(QString("%1x%1").arg(i + 1), i + 1);
    }

    //
    QStringList headers;
    headers << "";
    headers << GET_TEXT("CUSTOMLAYOUT/110003", "Layout Name");
    headers << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headers);
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setColumnCount(headers.size());
    ui->tableView->setSortingEnabled(false);
    ui->tableView->hideColumn(ColumnCheck);
    //
    ItemDelegateCustomLayout *itemDelegate = new ItemDelegateCustomLayout(this);
    connect(itemDelegate, SIGNAL(editintFinished(int, int, QString, QString)), this, SLOT(onCustomLayoutTableNameEditingFinished(int, int, QString, QString)));
    ui->tableView->setItemDelegateForColumn(ColumnName, itemDelegate);
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //
    ui->tableView->setColumnWidth(ColumnName, 150);
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onCustomLayoutTableClicked(int, int)));
    connect(ui->tableView, SIGNAL(itemDoubleClicked(int, int)), this, SLOT(onCustomLayoutTableDoubleClicked(int, int)));

    ui->tableView->setEditTriggers(QTableView::DoubleClicked);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

CustomLayoutDialog::~CustomLayoutDialog()
{
    delete ui;
}

void CustomLayoutDialog::initializeData(int screen, const CustomLayoutInfoList &layouts)
{
    m_screen = screen;
    m_allLayouts = layouts;
    showCustomLayoutTable();
    if (ui->tableView->rowCount() > 1) {
        ui->tableView->selectRow(0);
        setCurrentLayout(0);
    } else {
        m_scene->clear();
    }
}

const CustomLayoutInfoList &CustomLayoutDialog::allLayouts() const
{
    return m_allLayouts;
}

void CustomLayoutDialog::pushInfo(const CustomLayoutInfo &info)
{
    m_currentUndoStack->push(new CustomLayoutCommand(this, info));

    updateUndoRedoButtonState();
}

void CustomLayoutDialog::editBaseRowColumn(int row, int column)
{
    Q_UNUSED((column))

    ui->comboBoxBasic->beginEdit();
    ui->comboBoxBasic->setCurrentIndexFromData(row);
    ui->comboBoxBasic->endEdit();
}

void CustomLayoutDialog::editCustomLayoutInfo(const CustomLayoutInfo &info)
{
    m_allLayouts.replace(info);
    m_scene->setCustomLayoutInfo(info);
}

CustomLayoutScene *CustomLayoutDialog::scene()
{
    return m_scene;
}

void CustomLayoutDialog::showCustomLayoutTable()
{
    QStringList names = m_allLayouts.customLayoutNames(m_screen);

    ui->tableView->clearContent();
    ui->tableView->setRowCount(qMin(names.size() + 1, MaxCustomLayout));

    int row = 0;
    for (int i = 0; i < names.size(); ++i) {
        const auto &name = names.at(i);
        ui->tableView->setItemText(row, ColumnName, name);
        ui->tableView->setItemToolTip(row, ColumnName, name);
        row++;
    }

    if (row < 10) {
        ui->tableView->setItemPixmap(row, ColumnName, QPixmap(":/ptz/ptz/add.png"));
        ui->tableView->setItemText(row, ColumnDelete, "-");
    }

    ui->tableView->scrollToBottom();

    if (ui->tableView->rowCount() > 1) {
        ui->comboBoxBasic->setEnabled(true);
    } else {
        ui->comboBoxBasic->beginEdit();
        ui->comboBoxBasic->setCurrentIndexFromData(qMsNvr->maxChannel() == 64 ? 8 : 5);
        ui->comboBoxBasic->endEdit();
        ui->comboBoxBasic->setEnabled(false);

        m_currentUndoStack = nullptr;
        updateUndoRedoButtonState();
    }
}

void CustomLayoutDialog::addNewLayout()
{
    QMap<int, int> indexMap;
    for (int i = 0; i < m_allLayouts.size(); ++i) {
        const auto &info = m_allLayouts.at(i);
        if (info.screen() != m_screen) {
            continue;
        }
        QString name = info.name();
        QRegExp rx(R"(Custom Layout (\d+))");
        if (rx.exactMatch(name)) {
            int index = rx.cap(1).toInt();
            indexMap.insert(index, 0);
        }
    }
    QString name;
    for (int i = 1; i <= MaxCustomLayout; ++i) {
        if (indexMap.contains(i)) {
            continue;
        }
        name = QString("Custom Layout %1").arg(i);
        break;
    }
    CustomLayoutInfo info;
    int rowCount = qMsNvr->maxChannel() == 64 ? 8 : 5;
    info.addDefaultLayout(name, m_screen, rowCount, rowCount);
    m_allLayouts.append(info);
    showCustomLayoutTable();
    setCurrentLayout(name);
}

void CustomLayoutDialog::removeLayout(int row)
{
    QString name = ui->tableView->itemText(row, ColumnName);

    removeStack(name);

    CustomLayoutKey key(name, m_screen);
    m_allLayouts.remove(key);

    if (name == m_scene->currentName()) {
        m_scene->clear();
    }
}

void CustomLayoutDialog::cacheCurrentLayout()
{
    if (!m_scene->currentName().isEmpty()) {
        const CustomLayoutInfo &info = m_scene->customLayoutInfo();
        m_allLayouts.replace(info);
    }
}

void CustomLayoutDialog::removeStack(const QString &name)
{
    QUndoStack *stack = m_undoStackMap.value(name, nullptr);
    if (stack) {
        stack->deleteLater();
        m_undoStackMap.remove(name);
    }
}

void CustomLayoutDialog::renameStack(const QString &oldName, const QString &newName)
{
    QUndoStack *stack = m_undoStackMap.take(oldName);
    if (stack) {
        m_undoStackMap.insert(newName, stack);
    }
}

void CustomLayoutDialog::setCurrentLayout(const QString &name)
{
    int row = rowFromName(name);
    if (row < 0) {
        qMsWarning() << QString("name: %1, row: %2").arg(name).arg(row);
        return;
    }
    setCurrentLayout(row, name);
}

void CustomLayoutDialog::setCurrentLayout(int row)
{
    QString name = ui->tableView->itemText(row, ColumnName);
    setCurrentLayout(row, name);
}

void CustomLayoutDialog::setCurrentLayout(int row, const QString &name)
{
    if (name == m_scene->currentName()) {
        return;
    }

    ui->tableView->selectRow(row);
    const CustomLayoutInfo &currentInfo = m_allLayouts.find(CustomLayoutKey(name, m_screen));
    if (!currentInfo.isValid()) {
        qMsWarning() << "invalid customlayoutinfo";
    }
    ui->comboBoxBasic->beginEdit();
    ui->comboBoxBasic->setCurrentIndexFromData(currentInfo.baseRow());
    ui->comboBoxBasic->endEdit();
    m_scene->setCustomLayoutInfo(currentInfo);

    m_currentUndoStack = m_undoStackMap.value(name, nullptr);
    if (!m_currentUndoStack) {
        m_currentUndoStack = new QUndoStack(this);
        m_undoStackMap.insert(name, m_currentUndoStack);
    }
    updateUndoRedoButtonState();
}

int CustomLayoutDialog::rowFromName(const QString &name)
{
    if (name.isEmpty()) {
        return -1;
    }

    int rowCount = ui->tableView->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QString text = ui->tableView->itemText(i, ColumnName);
        if (text == name) {
            return i;
        }
    }
    return -1;
}

void CustomLayoutDialog::updateUndoRedoButtonState()
{
    if (!m_currentUndoStack) {
        ui->toolButtonUndo->setEnabled(false);
        ui->toolButtonUndo->setIcon(QIcon(":/layout/layout/undo_gray.png"));

        ui->toolButtonRedo->setEnabled(false);
        ui->toolButtonRedo->setIcon(QIcon(":/layout/layout/redo_gray.png"));
    } else {
        ui->toolButtonUndo->setEnabled(m_currentUndoStack->canUndo());
        if (ui->toolButtonUndo->isEnabled()) {
            ui->toolButtonUndo->setIcon(QIcon(":/layout/layout/undo_white.png"));
        } else {
            ui->toolButtonUndo->setIcon(QIcon(":/layout/layout/undo_gray.png"));
        }

        ui->toolButtonRedo->setEnabled(m_currentUndoStack->canRedo());
        if (ui->toolButtonRedo->isEnabled()) {
            ui->toolButtonRedo->setIcon(QIcon(":/layout/layout/redo_white.png"));
        } else {
            ui->toolButtonRedo->setIcon(QIcon(":/layout/layout/redo_gray.png"));
        }
    }
}

void CustomLayoutDialog::copyScreenLayout(int src, int dest)
{
    CustomLayoutInfoList srcList = m_allLayouts.infos(src);
    CustomLayoutInfoList destList = m_allLayouts.takeInfos(dest);
    for (int i = 0; i < srcList.size(); ++i) {
        CustomLayoutInfo srcInfo = srcList.at(i);
        const CustomLayoutInfo &destInfo = destList.find(srcInfo.key());
        srcInfo.setScreen(dest);
        if (destInfo.isValid()) {
            srcInfo.setChannels(destInfo.channels());
        } else {
            srcInfo.resetChannels();
        }
        m_allLayouts.append(srcInfo);
    }
}

void CustomLayoutDialog::onLanguageChanged()
{
    ui->toolButtonUndo->setToolTip(GET_TEXT("CUSTOMLAYOUT/110005", "Undo"));
    ui->toolButtonRedo->setToolTip(GET_TEXT("CUSTOMLAYOUT/110006", "Redo"));

    ui->label_title->setText(GET_TEXT("CUSTOMLAYOUT/110002", "Custom Layout"));
    ui->labelBasic->setText(GET_TEXT("CUSTOMLAYOUT/110004", "Basic Layout"));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void CustomLayoutDialog::onCustomLayoutTableClicked(int row, int column)
{
    int rowCount = ui->tableView->rowCount();
    qMsDebug() << QString("rowCount: %1, row: %2, column:%3").arg(rowCount).arg(row).arg(column);
    switch (column) {
    case ColumnName:
        if (ui->tableView->itemText(row, ColumnName).isEmpty()) {
            addNewLayout();
        } else {
            setCurrentLayout(row);
        }
        break;
    case ColumnDelete:
        if (ui->tableView->itemText(row, ColumnDelete).isEmpty()) {
            //删除后选中上一个
            removeLayout(row);
            showCustomLayoutTable();
            rowCount--;
            if (rowCount > 1) {
                if (row == 0) {
                    setCurrentLayout(0);
                } else {
                    setCurrentLayout(row - 1);
                }
            }
        } else {
        }
        break;
    default:
        break;
    }
}

void CustomLayoutDialog::onCustomLayoutTableDoubleClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
}

void CustomLayoutDialog::onCustomLayoutTableNameEditingFinished(int row, int column, const QString &oldText, const QString &newText)
{
    qMsDebug() << QString("row: %1, column: %2, old: %3, new: %4").arg(row).arg(column).arg(oldText).arg(newText);
    if (oldText == newText) {
        return;
    }

    if (newText.isEmpty()) {
        MessageBox::information(this, GET_TEXT("CUSTOMLAYOUT/110001", "Layout name can not be empty."));
        ui->tableView->setItemText(row, column, oldText);
        ui->tableView->edit(row, column);
        return;
    }

    CustomLayoutKey oldKey(oldText, m_screen, CustomLayoutKey::CustomType);
    CustomLayoutKey newKey(newText, m_screen, CustomLayoutKey::CustomType);
    if (m_allLayouts.contains(newKey)) {
        MessageBox::information(this, GET_TEXT("CUSTOMLAYOUT/110000", "The name has already existed."));
        ui->tableView->setItemText(row, column, oldText);
        ui->tableView->edit(row, column);
        return;
    } else {
        ui->tableView->setItemToolTip(row, column, newText);
        m_scene->setNewName(newText);

        renameStack(oldText, newText);

        bool ok = m_allLayouts.rename(oldKey, newText);
        if (!ok) {
            qMsWarning() << QString("row: %1, column: %2, oldText: %3, newText: %4").arg(row).arg(column).arg(oldText).arg(newText);
        }
    }
}

void CustomLayoutDialog::on_comboBoxBasic_indexSet(int index)
{
    if (ui->tableView->rowCount() <= 1) {
        m_scene->clear();
        return;
    }

    int row = index + 1;
    int column = index + 1;
    qMsDebug() << QString("base: %1x%2").arg(row).arg(column);
    CustomLayoutInfo info = m_scene->customLayoutInfo();
    info.addDefaultLayout(info.name(), m_screen, row, column);
    m_currentUndoStack->push(new CustomLayoutCommand(this, info));
    updateUndoRedoButtonState();
}

void CustomLayoutDialog::on_toolButtonUndo_clicked()
{
    m_currentUndoStack->undo();
    updateUndoRedoButtonState();
}

void CustomLayoutDialog::on_toolButtonRedo_clicked()
{
    m_currentUndoStack->redo();
    updateUndoRedoButtonState();
}

void CustomLayoutDialog::on_pushButtonOk_clicked()
{
    switch (m_screen) {
    case SCREEN_MAIN:
        copyScreenLayout(SCREEN_MAIN, SCREEN_SUB);
        break;
    case SCREEN_SUB:
        copyScreenLayout(SCREEN_SUB, SCREEN_MAIN);
        break;
    default:
        qMsWarning() << "invalid screen:" << m_screen;
        break;
    }

    accept();
    emit saved();
}

void CustomLayoutDialog::on_pushButtonCancel_clicked()
{
    close();
}
