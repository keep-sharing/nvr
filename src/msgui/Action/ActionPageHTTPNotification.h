#ifndef ACTIONPAGEHTTPNOTIFICATION_H
#define ACTIONPAGEHTTPNOTIFICATION_H

#include "ActionPageAbstract.h"

namespace Ui {
class ActionPageHTTPNotification;
}

class ActionPageHTTPNotification : public ActionPageAbstract
{
    Q_OBJECT

public:
    explicit ActionPageHTTPNotification(QWidget *parent = nullptr);
    ~ActionPageHTTPNotification() override;
    void dealMessage(MessageReceive *message) override;

protected:
    int loadData() override;
    int saveData() override;

private slots:
    void onLanguageChanged() override;
    void onScheduleTypeClicked(int id);
    void on_textEditURL_textChanged();

private:
    Ui::ActionPageHTTPNotification *ui;
    MyButtonGroup *m_group = nullptr;
};

#endif // ACTIONPAGEHTTPNOTIFICATION_H
