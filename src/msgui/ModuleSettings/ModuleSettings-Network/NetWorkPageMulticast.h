#ifndef NETWORKPAGEMULTICAST_H
#define NETWORKPAGEMULTICAST_H

#include "AbstractNetworkPage.h"
extern "C" {
#include "msdb.h"
}
namespace Ui {
class NetWorkPageMulticast;
}

class NetWorkPageMulticast : public AbstractNetworkPage
{
    Q_OBJECT

public:
    explicit NetWorkPageMulticast(QWidget *parent = nullptr);
    ~NetWorkPageMulticast();
    void initializeData() override;

    void processMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_SET_NETWORK_MULTICAST(MessageReceive *message);
private:
    QString ipAdjust(QString str);

private slots:
    void onLanguageChanged() override;
    void on_comboBoxEnable_activated(int index);

    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::NetWorkPageMulticast *ui;
};

#endif // NETWORKPAGEMULTICAST_H
