#ifndef RESETPASSWORD_H
#define RESETPASSWORD_H

#include "BaseShadowDialog.h"
#include <QTimer>

extern "C" {
#include "msdefs.h"
}

class MsWaitting;

namespace Ui {
class ResetPassword;
}

class ResetPassword : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit ResetPassword(QWidget *parent = nullptr);
    ~ResetPassword();

    static ResetPassword *instance();

    bool initializeData(QString userName = nullptr);

    void processMessage(MessageReceive *message);

protected:
    void ON_RESPONSE_FLAG_CHECK_SQA_LOCK(MessageReceive *message);
    void ON_RESPONSE_FLAG_CHECKSQA_LOCK(MessageReceive *message);

    void showEvent(QShowEvent *event) override;

private:
    QString getQuestion(int index);
    void initResetUnlockPattern(QString userName);

private slots:
    void onLanguageChanged();

    void on_pushButton_next_clicked();
    void on_pushButton_cancel_clicked();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_2_clicked();

    void on_pushButton_OK_clicked();
    void on_pushButton_Cancel_clicked();
    void onDrawStart();
    void onDrawFinished(QString text);
    void on_pushButton_Cancel_2_clicked();
    void on_pushButton_Next_clicked();

private:
    Ui::ResetPassword *ui;
    static ResetPassword *s_resetPassword;

    struct squestion m_sqa[3];

    QTimer *m_errorTimer = nullptr;
    int m_errorCount = 0;

    MsWaitting *m_waitting;
    QString m_pattern_text;
    int step = 1;
};

#endif // RESETPASSWORD_H
