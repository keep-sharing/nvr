#ifndef ACTIONPAGEWHITELED_H
#define ACTIONPAGEWHITELED_H

#include "ActionPageAbstract.h"
#include <QMap>

extern "C" {
#include "msdb.h"
}

class  AddWhiteLed;

namespace Ui {
class ActionPageWhiteLed;
}

class ActionPageWhiteLed : public ActionPageAbstract {
    Q_OBJECT

    enum TableLedColumn {
        LedCheck,
        LedChannel,
        LedMode,
        LedTime,
        LedEdit,
        LedDelete
    };

public:
    explicit ActionPageWhiteLed(QWidget *parent = nullptr);
    ~ActionPageWhiteLed();

    void dealMessage(MessageReceive *message) override;

protected:
    int loadData() override;
    int saveData() override;

private:
    void updateWhiteLedTable();

private slots:
    void onLanguageChanged() override;
    void onScheduleTypeClicked(int id);
    void onTableViewWhiteLedClicked(int row, int column);

private:
    Ui::ActionPageWhiteLed *ui;

    MyButtonGroup *m_group = nullptr;
    AddWhiteLed *m_addAction = nullptr;

    QMap<int, white_led_params> m_whiteLedParamsMap;
};

#endif // ACTIONPAGEWHITELED_H
