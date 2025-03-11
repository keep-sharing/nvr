#ifndef TABBANDWIDTH_H
#define TABBANDWIDTH_H

#include "AbstractSettingTab.h"

class QTimer;

namespace Ui {
class TabBandWidth;
}

class TabBandWidth : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabBandWidth(QWidget *parent = 0);
    ~TabBandWidth();

    void initializeData();

    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_GET_NETWORK_BANDWIDTH(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_NETWORK_SPEED(MessageReceive *message);

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

signals:
    void sig_back();

private slots:
    void onLanguageChanged();
    void onTimer();

    void on_pushButtonBack_clicked();

private:
    Ui::TabBandWidth *ui;

    QTimer *m_timer;
    int m_requestCount = 60;
};

#endif // TABBANDWIDTH_H
