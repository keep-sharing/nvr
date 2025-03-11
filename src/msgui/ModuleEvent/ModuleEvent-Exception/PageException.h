#ifndef PAGEEXCEPTION_H
#define PAGEEXCEPTION_H

#include "AbstractSettingPage.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class PageException;
}

class PageException : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PageException(QWidget *parent = nullptr);
    ~PageException();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void showAlarmOut(int value);
    void getAlarmOut(int &value);

private slots:
    void onLanguageChanged() override;

    void onSaveSetting();

    void onAlarmOutCheckChanged();
    void onAlarmOutAllClicked(bool checked);

    void on_comboBox_exceptionType_activated(int index);
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_comboBox_email_interval_activated(int index);

    void on_comboBoxAudioFile_activated(int index);

    void on_checkBoxHTTP_clicked();

    void on_textEditURL_textChanged();

    void on_comboBoxEnable_activated(int index);

  private:
    Ui::PageException *ui;

    trigger_alarms m_alarm;
    HTTP_NOTIFICATION_PARAMS_S m_httpPrms[EXCEPT_COUNT];
};

#endif // PAGEEXCEPTION_H
