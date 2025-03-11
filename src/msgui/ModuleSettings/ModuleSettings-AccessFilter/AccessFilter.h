#ifndef ACCESSFILTER_H
#define ACCESSFILTER_H

#include "AbstractSettingPage.h"

extern "C" {
#include "msstd.h"
}

namespace Ui {
class AccessFilter;
}

class AccessFilter : public AbstractSettingPage {
    Q_OBJECT

public:
    enum FilterColumn {
        ColumnCheck,
        ColumnAddress,
        ColumnEdit,
        ColumnDelete
    };
    enum AddressType {
        Mac,
        IpSingle,
        IpRange,
        None
    };
    explicit AccessFilter(QWidget *parent = 0);
    ~AccessFilter();

    virtual void initializeData() override;
    void processMessage(MessageReceive *message) override;

    void gotoAccessFilterPage();
    void deleteRow(int row);

private slots:
    void onLanguageChanged() override;

    //    void on_comboBox_filterType_activated(int index);
    void onTableItemClicked(int row, int column);

    void on_pushButton_add_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::AccessFilter *ui;
    QList<access_list> m_filterList;
};

#endif // ACCESSFILTER_H
