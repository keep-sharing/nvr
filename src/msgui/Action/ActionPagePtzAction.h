#ifndef ACTIONPAGEPTZACTION_H
#define ACTIONPAGEPTZACTION_H

#include "ActionPageAbstract.h"
#include <QMap>

extern "C" {
#include "msdb.h"
}

class AddPtzAction;

namespace Ui {
class ActionPagePtzAction;
}

class ActionPagePtzAction : public ActionPageAbstract {
    Q_OBJECT

    enum ptzActionColumn {
        ColumnCheck,
        ColumnChannel,
        ColumnActionType,
        ColumnNo,
        ColumnEdit,
        ColumnDelete
    };

public:
    explicit ActionPagePtzAction(QWidget *parent = nullptr);
    ~ActionPagePtzAction();

protected:
    int loadData() override;
    int saveData() override;

private:
    void updatePtzActionTable();

private slots:
    void onLanguageChanged() override;

    void onScheduleTypeClicked(int id);
    void onTableViewPtzActionClicked(int row, int column);

private:
    Ui::ActionPagePtzAction *ui;

    MyButtonGroup *m_group = nullptr;

    QMap<int, ptz_action_params> m_ptzActionMap;
    AddPtzAction *m_addAction = nullptr;
};

#endif // ACTIONPAGEPTZACTION_H
