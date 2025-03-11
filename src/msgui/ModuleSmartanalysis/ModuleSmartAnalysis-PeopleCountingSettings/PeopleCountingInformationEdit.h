#ifndef PEOPLECOUNTINGINFORMATIONEDIT_H
#define PEOPLECOUNTINGINFORMATIONEDIT_H

#include "BaseShadowDialog.h"
extern "C" {
#include "msg.h"
}
namespace Ui {
class PeopleCountingInformationEdit;
}

class PeopleCountingInformationEdit : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit PeopleCountingInformationEdit(QWidget *parent = nullptr);
    ~PeopleCountingInformationEdit();
    void initializeData(ms_smart_event_people_cnt *peopleCountInfo, int lineIndex);

    void processMessage(MessageReceive *message) override;
private:
    void ON_RESPONSE_FLAG_SET_VAC_CLEANCOUNT(MessageReceive *message);
    //sum<<3  Capacity<<2
    quint64 osdTypeChange(quint64 osdType);

private slots:
    void onLanguageChanged() override;
    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

    void on_pushButtonResetCountingInformation_clicked();
    void on_pushButtonResetCountingData_clicked();

    void on_toolButtonResetCountingInformation_clicked(bool checked);
    void on_toolButtonResetCountingData_clicked(bool checked);

private:
    Ui::PeopleCountingInformationEdit *ui;
    ms_smart_event_people_cnt *m_peopleCountInfo = nullptr;
    ms_smart_event_people_cnt *m_peopleCountInfoCache = nullptr;
    int m_currentLine = 0;

    QEventLoop m_eventLoop;
};

#endif // PEOPLECOUNTINGINFORMATIONEDIT_H
