#ifndef PAGENETWORK_H
#define PAGENETWORK_H

#include "AbstractSettingTab.h"

namespace Ui {
class TabNetwork;
}

class TabNetwork : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabNetwork(QWidget *parent = 0);
    ~TabNetwork();

    void initializeData();

    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_GET_NETWORK_INFO(MessageReceive *message);

signals:
    void sig_back();

private slots:
    void onLanguageChanged();

    void on_pushButtonBack_clicked();

private:
    Ui::TabNetwork *ui;
};

#endif // PAGENETWORK_H
