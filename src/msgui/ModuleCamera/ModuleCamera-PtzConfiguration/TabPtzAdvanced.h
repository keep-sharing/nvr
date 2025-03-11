#ifndef TABPTZADVANCED_H
#define TABPTZADVANCED_H

#include <QWidget>
#include <QMap>
#include "ptzbasepage.h"

extern "C"
{
#include "msdb.h"
}

namespace Ui {
class PtzAdvancedPage;
}

class TabPtzAdvanced : public PtzBasePage
{
    Q_OBJECT

public:
    explicit TabPtzAdvanced(QWidget *parent = nullptr);
    ~TabPtzAdvanced();

    void initializeData() override;

private:
    void copyData();
    bool isAdvancedDataChanged(int channel);

private slots:
    void onLanguageChanged();

    void on_comboBox_channel_activated(int index);
    void on_comboBox_connectionType_activated(int index);
    void on_comboBox_baudRate_activated(int index);
    void on_comboBox_dataBit_activated(int index);
    void on_comboBox_stopBit_activated(int index);
    void on_comboBox_checksumBit_activated(int index);
    void on_comboBox_protocol_activated(int index);
    void on_comboBox_address_activated(int index);

    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::PtzAdvancedPage *ui;

    //
    int m_advancedChannel = 0;
    QMap<int, struct ptz_port> m_ptzPortMap;
    QMap<int, struct ptz_port> m_ptzPortMapSource;
    QList<int> m_copyList;
};

#endif // TABPTZADVANCED_H
