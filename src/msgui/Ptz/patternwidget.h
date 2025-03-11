#ifndef PATTERNWIDGET_H
#define PATTERNWIDGET_H

#include "patternitemdelegate.h"
#include <QWidget>

extern "C" {
#include "msdefs.h"
}

class QTreeWidgetItem;

namespace Ui {
class PatternWidget;
}

class PatternWidget : public QWidget {
    Q_OBJECT

public:
    explicit PatternWidget(QWidget *parent = 0);
    ~PatternWidget();

    void initializeData(const int *pattern_array, int count = PATTERN_MAX);
    void clear();

    void clearPlayingState();

private:
    void updateItemButton(int row, int column);

signals:
    //index: 0-3
    void sendPatternControl(int action, int index);

private slots:
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void itemButtonClicked(int row, int index);

private:
    Ui::PatternWidget *ui;

    PatternItemDelegate *m_itemDelegate;
};

#endif // PATTERNWIDGET_H
