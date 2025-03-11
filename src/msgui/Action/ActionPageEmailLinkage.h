#ifndef ACTIONPAGEEMAILLINKAGE_H
#define ACTIONPAGEEMAILLINKAGE_H

#include "ActionPageAbstract.h"

class ChannelCheckBox;

namespace Ui {
class ActionPageEmailLinkage;
}

class ActionPageEmailLinkage : public ActionPageAbstract
{
    Q_OBJECT

public:
    explicit ActionPageEmailLinkage(QWidget *parent = nullptr);
    ~ActionPageEmailLinkage();
    void dealMessage(MessageReceive *message) override;

protected:
    int loadData() override;
    int saveData() override;

private slots:
    void onLanguageChanged() override;
    void onScheduleTypeClicked(int id);
    void onSnapshotCheckBoxChecked(int channel, bool checked);

private:
    Ui::ActionPageEmailLinkage *ui;

    MyButtonGroup *m_group = nullptr;
    QList<ChannelCheckBox *> m_checkBoxList;
};

#endif // ACTIONPAGEEMAILLINKAGE_H
