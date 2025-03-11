#ifndef ACTIONPAGEAUDIBLEWARNING_H
#define ACTIONPAGEAUDIBLEWARNING_H

#include "ActionPageAbstract.h"

namespace Ui {
class ActionPageAudibleWarning;
}

class ActionPageAudibleWarning : public ActionPageAbstract {
    Q_OBJECT

public:
    explicit ActionPageAudibleWarning(QWidget *parent);
    ~ActionPageAudibleWarning();

    void dealMessage(MessageReceive *message) override;

protected:
    int loadData() override;
    int saveData() override;

private slots:
    void onLanguageChanged() override;
    void onScheduleTypeClicked(int id);

private:
    Ui::ActionPageAudibleWarning *ui;

    MyButtonGroup *m_group = nullptr;
};

#endif // ACTIONPAGEAUDIBLEWARNING_H
