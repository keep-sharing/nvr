#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "BaseShadowDialog.h"

class MessageReceive;

namespace Ui {
class Authentication;
}

class Authentication : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit Authentication(QWidget *parent = 0);
    ~Authentication();

signals:
    void changed();

private slots:
    void onLanguageChanged() override;

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::Authentication *ui;

    bool isInputValid();
    QList<int> m_chnAuthCheckedList;

public:
    void append_edit_authentication(int channel);
    void initializeData();
};

#endif // AUTHENTICATION_H
