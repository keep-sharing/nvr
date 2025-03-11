#ifndef ACTIONPAGEOTHERS_H
#define ACTIONPAGEOTHERS_H

#include "ActionPageAbstract.h"

namespace Ui {
class ActionPageOthers;
}

class ActionPageOthers : public ActionPageAbstract
{
    Q_OBJECT

public:
    explicit ActionPageOthers(QWidget *parent = nullptr);
    ~ActionPageOthers();

    void dealMessage(MessageReceive *message) override;

protected:
    int loadData() override;
    int saveData() override;

protected slots:
    void onLanguageChanged() override;

private:
    Ui::ActionPageOthers *ui;
};

#endif // ACTIONPAGEOTHERS_H
