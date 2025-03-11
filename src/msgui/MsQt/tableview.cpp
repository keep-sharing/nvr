#include "tableview.h"
#include "MyDebug.h"
#include <QApplication>
#include <QEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QScrollBar>

TableView::TableView(QWidget *parent)
    : QTableView(parent)
{
    //header
    m_headerView = new HeaderView(Qt::Horizontal, this);
    connect(m_headerView, SIGNAL(headerChecked(bool)), this, SLOT(onHeaderChecked(bool)));
    connect(m_headerView, SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(onHandleIndicatorChanged(int, Qt::SortOrder)));
    m_headerView->setHighlightSections(false);
    m_headerView->setStretchLastSection(true);
    setHorizontalHeader(m_headerView);
    m_headerView->setCheckable(true);
    //
    verticalHeader()->setVisible(false);

    //model
    m_itemModel = new StandardItemModel(this);
    setEditTriggers(NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAlternatingRowColors(true);

    //sort
    m_sortModel = new SortFilterProxyModel(this);
    m_sortModel->setSourceModel(m_itemModel);
    setSortingEnabled(true);
    setModel(m_sortModel);

    //
    horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    horizontalScrollBar()->installEventFilter(this);

    //delegate
    setItemDelegateForColumn(0, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonCheckBox, this));

    //
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));
}

void TableView::setRowCount(int rows)
{
    m_itemModel->setRowCount(rows);
}

int TableView::rowCount()
{
    return m_itemModel->rowCount();
}

void TableView::setColumnCount(int columns)
{
    m_itemModel->setColumnCount(columns);
}

int TableView::columnCount()
{
    return m_itemModel->columnCount();
}

void TableView::insertRow(int row)
{
    m_itemModel->insertRow(row);
}

void TableView::removeRow(int row)
{
    m_sortModel->removeRow(row);
}

void TableView::removeRows(int row, int count)
{
    m_sortModel->removeRows(row, count);
}

void TableView::clear()
{
    m_itemModel->clear();
}

void TableView::clearContent()
{
    m_itemModel->setRowCount(0);
    m_headerView->setCheckState(Qt::Unchecked);
}

void TableView::clearCheck()
{
    for (int i = 0; i < rowCount(); ++i) {
        if (!isRowEnable(i)) {
            continue;
        }
        setItemData(i, 0, false, ItemCheckedRole);
    }
}

int TableView::currentRow() const
{
    return currentIndex().row();
}

void TableView::edit(int row, int column)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    QTableView::edit(index);
}

void TableView::setItemText(int row, int column, const QString &text)
{
    if (rowCount() <= row) {
        m_itemModel->insertRow(row);
    }

    const QModelIndex &index = m_sortModel->index(row, column);
    m_sortModel->setData(index, text, Qt::DisplayRole);
}

QString TableView::itemText(int row, int column) const
{
    const QModelIndex &index = m_sortModel->index(row, column);
    return m_sortModel->data(index, Qt::DisplayRole).toString();
}

void TableView::setItemIntValue(int row, int column, int value)
{
    setItemText(row, column, QString::number(value));
    setItemData(row, column, value, SortIntRole);
}

int TableView::itemIntValue(int row, int column)
{
    return itemData(row, column, SortIntRole).toInt();
}

void TableView::setItemBytesValue(int row, int column, qint64 bytes)
{
    if (rowCount() <= row) {
        m_itemModel->insertRow(row);
    }

    QString text = bytesString(bytes);
    setItemText(row, column, text);
    setItemData(row, column, bytes, SortIntRole);
}

void TableView::setItemDiskBytesValue(int row, int column, qint64 bytes)
{
    if (rowCount() <= row) {
        m_itemModel->insertRow(row);
    }

    QString text = diskBytesString(bytes);
    setItemText(row, column, text);
    setItemData(row, column, bytes, SortIntRole);
}

void TableView::setItemPixmap(int row, int column, const QPixmap &pixmap)
{
    if (rowCount() <= row) {
        m_itemModel->insertRow(row);
    }

    setItemData(row, column, QVariant::fromValue(pixmap), PixmapRole);
}

void TableView::setItemToolTip(int row, int column, const QString &text)
{
    setItemData(row, column, text, Qt::ToolTipRole);
}

void TableView::setItemData(const QModelIndex &index, const QVariant &value, int role)
{
    m_sortModel->setData(index, value, role);
}

void TableView::setItemData(int row, int column, const QVariant &value, int role)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    setItemData(index, value, role);
}

void TableView::setRowData(int row, const QVariant &value, int role)
{
    for (int column = 0; column < columnCount(); ++column) {
        setItemData(row, column, value, role);
    }
}

QVariant TableView::itemData(const QModelIndex &index, int role)
{
    return m_sortModel->data(index, role);
}

QVariant TableView::itemData(int row, int column, int role)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    return m_sortModel->data(index, role);
}

void TableView::setItemChecked(int row, bool checked)
{
    setItemData(row, 0, checked, ItemCheckedRole);

    //
    int checkedCount = 0;
    for (int i = 0; i < rowCount(); ++i) {
        if (itemData(i, 0, ItemCheckedRole).toBool()) {
            checkedCount++;
        }
    }
    if (checkedCount == 0) {
        m_headerView->setCheckState(Qt::Unchecked);
    } else if (checkedCount == rowCount() || (checkedCount == rowCount() - 1 && hasAddButton())) {
        m_headerView->setCheckState(Qt::Checked);
    } else {
        m_headerView->setCheckState(Qt::PartiallyChecked);
    }
}

bool TableView::isItemChecked(int row)
{
    bool checked = itemData(row, 0, ItemCheckedRole).toBool();
    return checked;
}

void TableView::setItemChecked(int row, int column, bool checked)
{
    setItemData(row, column, checked, ItemCheckedRole);
}

bool TableView::isItemChecked(int row, int column)
{
    bool checked = itemData(row, column, ItemCheckedRole).toBool();
    return checked;
}

void TableView::setItemEnable(int row, int column, bool enable)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    const QModelIndex &sourceIndex = m_sortModel->mapToSource(index);
    QStandardItem *item = m_itemModel->itemFromIndex(sourceIndex);
    if (item) {
        item->setEnabled(enable);
    }
    //
    if (enable) {
        setItemData(index, 0, ItemEnabledRole);
    } else {
        setItemData(index, -12345, ItemEnabledRole);
    }

    //
    QWidget *widget = indexWidget(index);
    if (widget) {
        widget->setEnabled(enable);
    }
}

void TableView::setRowEnable(int row, bool enable)
{
    for (int i = 0; i < columnCount(); ++i) {
        setItemEnable(row, i, enable);
    }
}

bool TableView::isItemEnable(int row, int column)
{
    bool enable = true;
    const QModelIndex &index = m_sortModel->index(row, column);
    const QModelIndex &sourceIndex = m_sortModel->mapToSource(index);
    QStandardItem *item = m_itemModel->itemFromIndex(sourceIndex);
    if (item) {
        enable = item->isEnabled();
    }
    return enable;
}

bool TableView::isRowEnable(int row)
{
    return isItemEnable(row, 0);
}

void TableView::setCheckListEnable(bool enable)
{
    for (int i = 0; i < rowCount(); ++i) {
        setItemEnable(i, 0, enable);
    }
    setHeaderEnable(enable);
}

void TableView::setItemWidget(int row, int column, QWidget *widget)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    setIndexWidget(index, widget);
}

QWidget *TableView::itemWidget(int row, int column)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    return indexWidget(index);
}

void TableView::openPersistentEditor(int row, int column)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    QTableView::openPersistentEditor(index);
}

void TableView::closePersistentEditor(int row, int column)
{
    const QModelIndex &index = m_sortModel->index(row, column);
    QTableView::closePersistentEditor(index);
}

void TableView::setRowColor(int row, const QColor &color)
{
    for (int column = 0; column < columnCount(); ++column) {
        const QModelIndex &index = m_sortModel->index(row, column);
        m_sortModel->setData(index, color, Qt::ForegroundRole);
    }
}

/**
 * @brief TableView::firstVisibleRow
 * 第一行可见行
 * @return
 */
int TableView::firstVisibleRow() const
{
    int row = -1;
    const QModelIndex &index = indexAt(QPoint(0, 0));
    if (index.isValid()) {
        row = index.row();
    }
    return row;
}

/**
 * @brief TableView::scrollToRow
 * 把第row行作为第一行可见行
 * @param row
 */
void TableView::scrollToRow(int row)
{
    const QModelIndex &index = m_sortModel->index(row, 0);
    scrollTo(index, QAbstractItemView::PositionAtTop);
}

void TableView::setHorizontalHeaderLabels(const QStringList &strList)
{
    m_itemModel->setHorizontalHeaderLabels(strList);
}

void TableView::setHorizontalHeaderItem(int column, const QString &text)
{
    m_itemModel->setHorizontalHeaderItem(column, new QStandardItem(text));
}

void TableView::setResizeMode(int logicalIndex, QHeaderView::ResizeMode mode)
{
    m_headerView->setStretchLastSection(false);
    m_headerView->setResizeMode(logicalIndex, mode);
}

void TableView::setHeaderCheckable(bool enable)
{
    m_headerView->setCheckable(enable);
}

void TableView::setHeaderChecked(bool checked)
{
    if (checked) {
        m_headerView->setCheckState(Qt::Checked);
    } else {
        m_headerView->setCheckState(Qt::Unchecked);
    }
}

void TableView::setAllColumnWidth(const QList<int> widthList)
{
    if (widthList.isEmpty()) {
        int w = width() / columnCount();
        for (int i = 0; i < columnCount(); ++i) {
            setColumnWidth(i, w);
        }
    } else {
        for (int i = 0; i < widthList.size(); ++i) {
            setColumnWidth(i, widthList.at(i));
        }
    }
}

void TableView::setHeaderEnable(bool enable)
{
    m_headerView->setEnabled(enable);
}

void TableView::setSortType(int column, SortFilterProxyModel::SortType type)
{
    m_sortModel->setSortType(column, type);
}

void TableView::setSortableForColumn(int column, int enable)
{
    m_headerView->setSortableForColumn(column, enable);
    m_sortModel->setSortableForColumn(column, enable);
}

/**
 * @brief TableView::reorder
 * Qt4有个问题，新插入的无法排序，所以插入完了再重新排序一次
 */
void TableView::reorder()
{
#if QT_VERSION < 0x050000
    m_sortModel->sort(m_sortModel->sortColumn(), m_sortModel->sortOrder());
#endif
}
//用于页面初始化，清空排序以其标识
void TableView::clearSort()
{
    horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    m_sortModel->sort(0, Qt::AscendingOrder);
}

void TableView::setFilterFixedString(const QString &pattern)
{
    m_sortModel->setFilterFixedString(pattern);
}

QString TableView::bytesString(qint64 bytes)
{
    // According to the Si standard KB is 1000 bytes, KiB is 1024
    // but on windows sizes are calculated by dividing by 1024 so we do what they do.
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;
    if (bytes >= tb)
        return QString("%1 TB").arg(QLocale().toString(qreal(bytes) / tb - 0.0004, 'f', 3));
    if (bytes >= gb)
        return QString("%1 GB").arg(QLocale().toString(qreal(bytes) / gb - 0.004, 'f', 2));
    if (bytes >= mb)
        return QString("%1 MB").arg(QLocale().toString(qreal(bytes) / mb - 0.04, 'f', 1));
    if (bytes >= kb)
        return QString("%1 KB").arg(QLocale().toString(bytes / kb));
    return QString("%1 bytes").arg(QLocale().toString(bytes));
}

QString TableView::diskBytesString(qint64 bytes)
{
    // According to the Si standard KB is 1000 bytes, KiB is 1024
    // but on windows sizes are calculated by dividing by 1024 so we do what they do.
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;
    if (bytes == 0) {
        return QString("0");
    }
    if (bytes >= tb) {
        return QString("%1 TB").arg(QLocale().toString(qreal(bytes) / tb - 0.0004, 'f', 3));
    } else {
        return QString("%1 GB").arg(QLocale().toString(qreal(bytes) / gb - 0.004, 'f', 2));
    }
}

bool TableView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == horizontalScrollBar()) {
        switch (event->type()) {
        case QEvent::Wheel:
            return true;
            break;
        case QEvent::MouseMove:
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void TableView::contextMenuEvent(QContextMenuEvent *)
{
}

void TableView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QWidget::mousePressEvent(event);
    } else {
        QTableView::mousePressEvent(event);
    }
}

void TableView::onItemClicked(const QModelIndex &index)
{
    switch (index.column()) {
    case 0: {
        if (isRowEnable(index.row())) {
            bool checked = isItemChecked(index.row());
            setItemChecked(index.row(), !checked);
        }
        break;
    }
    default:
        break;
    }
    //
    emit itemClicked(index.row(), index.column());
}

void TableView::onItemDoubleClicked(const QModelIndex &index)
{
    emit itemDoubleClicked(index.row(), index.column());
}

void TableView::onHeaderChecked(bool checked)
{
    int checkedCount = 0;
    for (int i = 0; i < rowCount(); ++i) {
        if (!isRowEnable(i)) {
            continue;
        }

        //
        checkedCount++;
        setItemData(i, 0, checked, ItemCheckedRole);
    }
    if (checkedCount == rowCount() - 1 && hasAddButton()) {
        checkedCount++;
    }
    if (checked && checkedCount != rowCount()) {
        m_headerView->setCheckState(Qt::PartiallyChecked);
    }
    emit headerChecked(checked);
}

void TableView::onHandleIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    Q_UNUSED(order)
    if (!m_headerView->sortableForColumn(logicalIndex) && m_headerView->sortableForColumn(m_sortModel->sortColumn())) {
        horizontalHeader()->setSortIndicator(m_sortModel->sortColumn(), m_sortModel->sortOrder());
        m_sortModel->sort(m_sortModel->sortColumn(), m_sortModel->sortOrder());
    }
}

bool TableView::hasAddButton() const
{
    return m_hasAddButton;
}

void TableView::setHasAddButton(bool hasAddButton)
{
    m_hasAddButton = hasAddButton;
}
