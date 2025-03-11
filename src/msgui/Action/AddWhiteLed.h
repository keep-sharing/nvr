#ifndef ADDWHITELED_H
#define ADDWHITELED_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class AddWhiteLed;
}

class AddWhiteLed : public BaseShadowDialog
{
    Q_OBJECT

    enum Mode
    {
        ModeAdd,
        ModeEdit,
        ModeNone
    };

public:
    explicit AddWhiteLed(QWidget *parent = nullptr);
    ~AddWhiteLed();

    static AddWhiteLed *instance();

    void setParamsMap(const QMap<int, WHITE_LED_PARAMS> &map);

    int execAdd();
    int execEdit(const WHITE_LED_PARAMS &params);

    WHITE_LED_PARAMS params() const;

    void dealMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_GET_IPC_LED_PARAMS(MessageReceive *message);

private slots:
    void on_comboBox_channel_activated(int index);
    void on_comboBox_mode_indexSet(int index);
    void on_pushButton_reset_clicked();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    static AddWhiteLed *s_self;

    Ui::AddWhiteLed *ui;

    Mode m_mode = ModeNone;

    QMap<int, WHITE_LED_PARAMS> m_paramsMap;
    WHITE_LED_PARAMS m_params;
};

#endif // ADDWHITELED_H
