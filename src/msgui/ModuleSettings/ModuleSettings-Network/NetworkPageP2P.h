#ifndef NETWORKPAGEP2P_H
#define NETWORKPAGEP2P_H

#include "AbstractNetworkPage.h"
#include <QTimer>

extern "C" {
#include "msdb.h"
}

namespace Ui {
class NetworkPageP2P;
}

class NetworkPageP2P : public AbstractNetworkPage {
    Q_OBJECT

public:
    explicit NetworkPageP2P(QWidget *parent = nullptr);
    ~NetworkPageP2P();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_P2P_STATUS_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_ENABLE_P2P(MessageReceive *message);

private slots:
    void onLanguageChanged();

    void onTimeout();

    void on_comboBox_p2p_activated(int index);

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::NetworkPageP2P *ui;

    p2p_info m_dbP2P;

    QTimer *m_timer;
};

#endif // NETWORKPAGEP2P_H
