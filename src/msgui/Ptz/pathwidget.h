#ifndef PATHWIDGET_H
#define PATHWIDGET_H

#include "MsWidget.h"
#include "patroleditdelegate.h"
#include "patrolitemdelegate.h"
#include <QTableWidgetItem>

extern "C" {
#include "msg.h"
}

class QTreeWidgetItem;

namespace Ui {
class PathWidget;
}

class PathWidget : public MsWidget {
    Q_OBJECT

public:
    enum PatternOperation {
        PatternStartRecord,
        PatternStopRecord,
        PatternRun,
        PatternStop,
        PatternDelete
    };

    explicit PathWidget(QWidget *parent = 0);
    ~PathWidget();

    void initializeData(int channel, const resp_ptz_tour *tour_array, int count = TOUR_MAX);
    void clear();

    void clearPlayingState();

    void sendPatrolControl(int action, int tourid);

private:
    void sendPatrolData(int tourid);
    void sendFisheyePatrolData(int tourid);
    void updatePatrolDataFromTable();
    void updateItemButton(int row, int column);

signals:

private slots:
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void itemButtonClicked(int row, int index);

    void on_pushButton_add_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_up_clicked();
    void on_pushButton_down_clicked();

    void on_pushButton_save_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::PathWidget *ui;

    PatrolItemDelegate *m_itemDelegate;
    PatrolEditDelegate *m_patrolEditDelegate;

    int m_channel;

    QList<resp_ptz_tour> m_tourList;
    int m_currentPathIndex = 0; //0-7
};

#endif // PATHWIDGET_H
