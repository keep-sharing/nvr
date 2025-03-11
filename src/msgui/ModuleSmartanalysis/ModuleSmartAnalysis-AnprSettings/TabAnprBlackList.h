#ifndef TABANPRBLACKLIST_H
#define TABANPRBLACKLIST_H

#include "anprbasepage.h"
#include <QEventLoop>

class ActionAnpr;
class EffectiveTimeAnpr;

struct smart_event;

namespace Ui {
class AnprBlackListPage;
}

class TabAnprBlackList : public AnprBasePage {
    Q_OBJECT

public:
    explicit TabAnprBlackList(QWidget *parent = nullptr);
    ~TabAnprBlackList();

    void initializeData(int channel) override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_LPR_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_ANPR_EVENT(MessageReceive *message);

    void showEnable(bool enable);

private slots:
    void onLanguageChanged();

    void on_pushButton_effectiveTime_clicked();
    void on_pushButton_action_clicked();

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_checkBox_enable_stateChanged(int arg1);

private:
    Ui::AnprBlackListPage *ui;

    int m_currentChannel;
    smart_event *m_anprEvent = nullptr;

    EffectiveTimeAnpr *m_effectiveTime = nullptr;
    ActionAnpr *m_action = nullptr;

    QEventLoop m_eventLoop;
};

#endif // TABANPRBLACKLIST_H
