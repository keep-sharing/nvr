#ifndef ADDNETWORKDISK_H
#define ADDNETWORKDISK_H

#include "BaseShadowDialog.h"

class MsWaitting;

namespace Ui {
class AddNetworkDisk;
}

class AddNetworkDisk : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit AddNetworkDisk(QWidget *parent = nullptr);
    ~AddNetworkDisk();

    void setDiskPort(int port);

    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_SEARCH_MSFS_NAS(MessageReceive *message);
    void ON_RESPONSE_FLAG_ADD_MSFS_NAS(MessageReceive *message);

private slots:
    void onLanguageChanged();
    void onTableItemClicked(int row, int column);

    void on_comboBox_type_indexSet(int index);

    void on_pushButton_search_clicked();
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::AddNetworkDisk *ui;

    MsWaitting *m_waitting;

    int m_port = -1;
};

#endif // ADDNETWORKDISK_H
