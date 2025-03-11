#ifndef ACTIONPAGEEVENTPOPUP_H
#define ACTIONPAGEEVENTPOPUP_H

#include "ActionPageAbstract.h"

namespace Ui {
class ActionPageEventPopup;
}

class ActionPageEventPopup : public ActionPageAbstract
{
    Q_OBJECT

public:
    explicit ActionPageEventPopup(QWidget *parent = nullptr);
    ~ActionPageEventPopup();

    void dealMessage(MessageReceive *message) override;

protected:
    int loadData() override;
    int saveData() override;

private slots:
    void onLanguageChanged() override;
    void onScheduleTypeClicked(int id);
    void onComboBoxNumbersOfChannelIndexSet(int index);

private:
    Ui::ActionPageEventPopup *ui;

    MyButtonGroup *m_group = nullptr;
};

#endif // ACTIONPAGEEVENTPOPUP_H
