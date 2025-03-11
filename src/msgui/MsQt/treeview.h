#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>
#include <QStyledItemDelegate>
#include "sortfilterproxymodel.h"
#include "standarditemmodel.h"
#include "headerview.h"
#include "itembuttondelegate.h"
#include "itemchanneldelegate.h"
#include "itemcheckedbuttondelegate.h"

class TreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit TreeView(QWidget *parent = 0);

    //header
    void setHorizontalHeaderLabels(const QStringList &strList);
    void setHorizontalHeaderCheckable(bool checkable);

    void clear();
    void clearContent();
    void clearCheck();
    void setRowCount(int rows);
    void setColumnCount(int columns);
    int rowCount();
    int columnCount();
    void selectPreviousRow();
    void selectNextRow();

    void setCurrentRow(int row);

    void setItemText(int row, int column, const QString &text);
    QString itemText(int row, int column);
    void setItemToolTip(int row, int column, const QString &text);
    void setItemData(int row, int column, const QVariant &value, int role);
    void setItemData(const QModelIndex &index, const QVariant &value, int role);
    QVariant itemData(int row, int column, int role) const;
    QVariant itemData(const QModelIndex &index, int role) const;
    //channel
    void setItemChannel(int row, int channel);
    int itemChannel(const QModelIndex &index) const;
    int itemChannel(int row) const;

    void setItemColorText(int row, int column, const QString &text, const QString &color = "#FFFFFF");
    void setItemColor(int row, int column, const QString &color);
    void setRowColor(int row, const QString &color);
    void clearItemColor();
    void clearRowColor(int row);

    void setItemChecked(int row, bool checked);
    bool isItemChecked(int row);
    int firstCheckedRow();
    void clearItemChecked();

    void openPersistentEditor(int row, int column);

protected:
    void contextMenuEvent(QContextMenuEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void headerChecked(bool checked);
    void itemClicked(int row, int column);
    void enterPressed();

private slots:
    void onHeaderChecked(bool checked);

    void onClicked(const QModelIndex &index);

private:
    HeaderView *m_headerView;
    SortFilterProxyModel *m_sortModel;
    StandardItemModel *m_itemModel;

    bool m_checkable = true;
    int m_currentRow = -1;

    bool m_isSelectedWhenChecked = false;
};

#endif // TREEVIEW_H
