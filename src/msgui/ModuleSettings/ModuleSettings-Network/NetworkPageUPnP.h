#ifndef NETWORKPAGEUPNP_H
#define NETWORKPAGEUPNP_H

#include "AbstractNetworkPage.h"
#include "itembuttonwidget.h"
#include <QTimer>

class UPnPEdit;

namespace Ui {
class NetworkPageUPnp;
}

class NetworkPageUPnp : public AbstractNetworkPage {
    Q_OBJECT

public:
    enum UpnpColumn {
        ColumnType,
        ColumnEdit,
        ColumnExternal,
        ColumnInternal,
        ColumnStatus
    };

public:
    explicit NetworkPageUPnp(QWidget *parent = nullptr);
    ~NetworkPageUPnp();

    void initializeData() override;

    void initNetUPnPTab();
    void gotoNetUPnPtab();
    void freeNetUPnPtab();
    void readUPnPConfig();
    bool saveUPnPConfig();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLanguageChanged();

    void slotRefreshUPnPTable();

    void onItemClicked(int row, int column);
    void onItemDoubleClicked(int row, int column);

    void slotEnableUPnP(int index);
    void slotTypeUPnP(int index);

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::NetworkPageUPnp *ui;

    struct upnp *pUPnP_Db;
    struct upnp *pUPnP_DbOri;
    struct network_more *pNetPortDb;
    UPnPEdit *pUPnPEidtDlg;
    QTimer *refreshTimer;
};

#endif // NETWORKPAGEUPNP_H
