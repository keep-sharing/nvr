#include "treeview.h"
#include <QApplication>
#include <QMouseEvent>
#include <QtDebug>

TreeView::TreeView(QWidget *parent) :
    QTreeView(parent)
{
    m_headerView = new HeaderView(Qt::Horizontal, this);
    connect(m_headerView, SIGNAL(headerChecked(bool)), this, SLOT(onHeaderChecked(bool)));
    m_headerView->setHighlightSections(false);
    m_headerView->setStretchLastSection(true);
    setHeader(m_headerView);

    m_itemModel = new StandardItemModel(this);
    m_sortModel = new SortFilterProxyModel(this);
    m_sortModel->setSourceModel(m_itemModel);
    setModel(m_sortModel);

    setIndentation(0);
    setAlternatingRowColors(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
}

void TreeView::setHorizontalHeaderLabels(const QStringList &strList)
{
    m_itemModel->setHorizontalHeaderLabels(strList);
}

void TreeView::setHorizontalHeaderCheckable(bool checkable)
{
    m_headerView->setCheckable(checkable);
}

void TreeView::clear()
{
    m_itemModel->clear();
}

void TreeView::clearContent()
{
    m_itemModel->setRowCount(0);
    m_headerView->setCheckState(Qt::Unchecked);
}

void TreeView::clearCheck()
{
    for (int i = 0; i < rowCount(); ++i)
    {
        setItemData(i, 0, false, ItemCheckedRole);
    }
    m_headerView->setCheckState(Qt::Unchecked);
}

void TreeView::setRowCount(int rows)
{
    m_itemModel->setRowCount(rows);
}

void TreeView::setColumnCount(int columns)
{
    m_itemModel->setColumnCount(columns);
}

int TreeView::rowCount()
{
    return m_itemModel->rowCount();
}

int TreeView::columnCount()
{
    return m_itemModel->columnCount();
}

void TreeView::selectPreviousRow()
{
    int row = currentIndex().row();
    if (row < 0 || row >= rowCount())
    {
        row = 0;
    }
    else if (row == 0)
    {
        row = rowCount() - 1;
    }
    else
    {
        row--;
    }
    setCurrentRow(row);
}

void TreeView::selectNextRow()
{
    int row = currentIndex().row();
    if (row < 0 || row >= rowCount())
    {
        row = 0;
    }
    else if (row == rowCount() - 1)
    {
        row = 0;
    }
    else
    {
        row++;
    }
    setCurrentRow(row);
}

void TreeView::setCurrentRow(int row)
{
    const QModelIndex &index = m_sortModel->index(row, 0);
    setCurrentIndex(index);
}

void TreeView::setItemText(int row, int column, const QString &text)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    m_sortModel->setData(index, text, Qt::DisplayRole);
}

QString TreeView::itemText(int row, int column)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    return m_sortModel->data(index, Qt::DisplayRole).toString();
}

void TreeView::setItemToolTip(int row, int column, const QString &text)
{
    setItemData(row, column, text, Qt::ToolTipRole);
}

void TreeView::setItemData(int row, int column, const QVariant &value, int role)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    setItemData(index, value, role);
}

void TreeView::setItemData(const QModelIndex &index, const QVariant &value, int role)
{
    m_sortModel->setData(index, value, role);
    //
    if (index.column() == 0 && role == ItemCheckedRole)
    {
        int checkedCount = 0;
        for (int i = 0; i < rowCount(); ++i)
        {
            if (itemData(i, 0, ItemCheckedRole).toBool())
            {
                checkedCount++;
            }
        }
        if (checkedCount == 0)
        {
            m_headerView->setCheckState(Qt::Unchecked);
        }
        else if (checkedCount == rowCount())
        {
            m_headerView->setCheckState(Qt::Checked);
        }
        else
        {
            m_headerView->setCheckState(Qt::PartiallyChecked);
        }
    }
}

QVariant TreeView::itemData(int row, int column, int role) const
{
    const QModelIndex &index = m_sortModel->index(row, column);
    return itemData(index, role);
}

QVariant TreeView::itemData(const QModelIndex &index, int role) const
{
    return m_sortModel->data(index, role);
}

void TreeView::setItemChannel(int row, int channel)
{
    setItemData(row, 0, channel, ItemChannelRole);
}

int TreeView::itemChannel(const QModelIndex &index) const
{
    return itemData(index.row(), 0, ItemChannelRole).toInt();
}

int TreeView::itemChannel(int row) const
{
    return itemData(row, 0, ItemChannelRole).toInt();
}

void TreeView::setItemColorText(int row, int column, const QString &text, const QString &color)
{
    setItemData(row, column, text, ItemTextRole);
    setItemData(row, column, color, ItemColorRole);
}

void TreeView::setItemColor(int row, int column, const QString &color)
{
    setItemData(row, column, color, ItemColorRole);
}

void TreeView::setRowColor(int row, const QString &color)
{
    for (int c = 0; c < columnCount(); ++c)
    {
        setItemData(row, c, color, ItemColorRole);
    }
}

void TreeView::clearItemColor()
{
    for (int r = 0; r < rowCount(); ++r)
    {
        for (int c = 0; c < columnCount(); ++c)
        {
            setItemData(r, c, "", ItemColorRole);
        }
    }
}

void TreeView::clearRowColor(int row)
{
    for (int c = 0; c < columnCount(); ++c)
    {
        setItemData(row, c, "", ItemColorRole);
    }
}

void TreeView::setItemChecked(int row, bool checked)
{
    setItemData(row, 0, checked, ItemCheckedRole);
}

bool TreeView::isItemChecked(int row)
{
    bool checked = itemData(row, 0, ItemCheckedRole).toBool();
    return checked;
}

int TreeView::firstCheckedRow()
{
    int row = -1;
    for (int r = 0; r < rowCount(); ++r)
    {
        if (isItemChecked(r))
        {
            row = r;
            break;
        }
    }
    return row;
}

void TreeView::clearItemChecked()
{
    for (int r = 0; r < rowCount(); ++r)
    {
        setItemChecked(r, false);
    }
}

void TreeView::openPersistentEditor(int row, int column)
{
    const QModelIndex &index = m_sortModel->mapFromSource(m_itemModel->index(row, column));
    QTreeView::openPersistentEditor(index);
}

void TreeView::contextMenuEvent(QContextMenuEvent *)
{

}

void TreeView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        QWidget::mousePressEvent(event);
    }
    else
    {
        QTreeView::mousePressEvent(event);
    }
}

void TreeView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        QWidget::mouseReleaseEvent(event);
    }
    else
    {
        QTreeView::mouseReleaseEvent(event);
    }
}

void TreeView::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "TreeView::keyPressEvent," << event;
    switch (event->key())
    {
    case Qt::Key_Up:
        selectPreviousRow();
        break;
    case Qt::Key_Down:
        selectNextRow();
        break;
    case Qt::Key_Return:    //网络键盘Enter
        emit enterPressed();
        break;
    default:
        break;
    }
}

void TreeView::onHeaderChecked(bool checked)
{
    for (int i = 0; i < rowCount(); ++i)
    {
        setItemData(i, 0, checked, ItemCheckedRole);
    }

    emit headerChecked(checked);
}

void TreeView::onClicked(const QModelIndex &index)
{
    emit itemClicked(index.row(), index.column());
}

