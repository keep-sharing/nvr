#ifndef NETWORKDISKEDIT_H
#define NETWORKDISKEDIT_H

#include "abstractdiskedit.h"

namespace Ui {
class NetworkDiskEdit;
}

class NetworkDiskEdit : public AbstractDiskEdit {
    Q_OBJECT

    enum Column {
        ColumnCheck,
        ColumnNo,
        ColumnDirectory
    };

public:
    explicit NetworkDiskEdit(QWidget *parent = nullptr);
    ~NetworkDiskEdit();

    void setDiskPort(int port);

    void processMessage(MessageReceive *message) override;

    bool isNoChange();

private:
    void ON_RESPONSE_FLAG_SEARCH_MSFS_NAS(MessageReceive *message);

private slots:
    void onLanguageChanged() override;

    void onTableItemClicked(int row, int column);

    void on_comboBox_type_indexSet(int index);

    void on_pushButton_search_clicked();
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::NetworkDiskEdit *ui;

    QString m_address;
    QString m_directory;
    int m_port = -1;
};

#endif // NETWORKDISKEDIT_H
