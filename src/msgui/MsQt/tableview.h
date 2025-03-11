#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>
#include <QStandardItem>
#include "standarditemmodel.h"
#include "sortfilterproxymodel.h"
#include "headerview.h"
#include "itemcheckedbuttondelegate.h"
#include "itembuttondelegate.h"
#include "itemicondelegate.h"

class TableView : public QTableView
{
    Q_OBJECT
public:
    explicit TableView(QWidget *parent = 0);

    void setRowCount(int rows);
    int rowCount();
    void setColumnCount(int columns);
    int columnCount();

    void insertRow(int row);

    void removeRow(int row);
    void removeRows(int row, int count);

    void clear();
    void clearContent();
    void clearCheck();

    int currentRow() const;

    void edit(int row, int column);

    void setItemText(int row, int column, const QString &text);
    QString itemText(int row, int column) const;
    //
    void setItemIntValue(int row, int column, int value);
    int itemIntValue(int row, int column);
    //GB MB bit
    void setItemBytesValue(int row, int column, qint64 bytes);
    void setItemDiskBytesValue(int row, int column, qint64 bytes);
    //pixmap
    void setItemPixmap(int row, int column, const QPixmap &pixmap);
    //
    void setItemToolTip(int row, int column, const QString &text);
    //
    void setItemData(const QModelIndex &index, const QVariant &value, int role);
    void setItemData(int row, int column, const QVariant &value, int role);
    void setRowData(int row, const QVariant &value, int role);
    QVariant itemData(const QModelIndex &index, int role);
    QVariant itemData(int row, int column, int role);
    //
    void setItemChecked(int row, bool checked);
    bool isItemChecked(int row);
    void setItemChecked(int row, int column, bool checked);
    bool isItemChecked(int row, int column);
    //
    void setItemEnable(int row, int column, bool enable);
    void setRowEnable(int row, bool enable);
    bool isItemEnable(int row, int column);
    bool isRowEnable(int row);
    void setCheckListEnable(bool enable);
    //
    void setItemWidget(int row, int column, QWidget *widget);
    QWidget *itemWidget(int row, int column);

    void openPersistentEditor(int row, int column);
    void closePersistentEditor(int row, int column);

    //
    void setRowColor(int row, const QColor &color);

    //
    int firstVisibleRow() const;
    void scrollToRow(int row);

    //header
    void setHorizontalHeaderLabels(const QStringList &strList);
    void setHorizontalHeaderItem(int column, const QString &text);
    void setResizeMode(int logicalIndex, QHeaderView::ResizeMode mode);
    void setHeaderCheckable(bool enable);
    void setHeaderChecked(bool checked);
    void setAllColumnWidth(const QList<int> widthList = QList<int>());
    void setHeaderEnable(bool enable);
    //sort
    void setSortType(int column, SortFilterProxyModel::SortType type);
    void setSortableForColumn(int column, int enable);
    void reorder();
    void clearSort();
    //filter
    void setFilterFixedString(const QString &pattern);

    static QString bytesString(qint64 bytes);
    static QString diskBytesString(qint64 bytes);

    bool hasAddButton() const;
    void setHasAddButton(bool hasAddButton);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void itemClicked(int row, int column);
    void itemDoubleClicked(int row, int column);
    void headerChecked(bool checked);

private slots:
    void onItemClicked(const QModelIndex &index);
    void onItemDoubleClicked(const QModelIndex &index);
    void onHeaderChecked(bool checked);
    void onHandleIndicatorChanged(int logicalIndex, Qt::SortOrder order);

protected:
    HeaderView *m_headerView;
    StandardItemModel *m_itemModel;
    SortFilterProxyModel *m_sortModel;

    bool m_hasAddButton = false;
    int m_sortColumn;
};

#endif // TABLEVIEW_H
