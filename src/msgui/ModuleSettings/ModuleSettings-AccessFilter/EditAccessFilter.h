#ifndef EDITACCESSFILTER_H
#define EDITACCESSFILTER_H

#include "AbstractSettingPage.h"
#include "BaseShadowDialog.h"
#include <QWidget>

extern "C" {
#include "msg.h"
}

namespace Ui {
class EditAccessFilter;
}

class EditAccessFilter : public BaseShadowDialog {
    Q_OBJECT

public:
    enum AddressType {
        Mac,
        IpSingle,
        IpRange,
        None
    };
    explicit EditAccessFilter(QWidget *parent = 0);
    ~EditAccessFilter();

    static EditAccessFilter *instance();
    void initializeData(int index, QList<access_list> filterList);
    QString getAddress() const;
    int isAddressRepeated(QString address, int isRange = 0);
    int check_ipv4_range_ok(QString ipStart, QString ipEnd);
    QString ipAdjust(QString str);

private slots:
    void on_comboBox_addressType_activated(int index);
    void on_comboBox_ipRule_activated(int index);
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();
    void onLanguageChanged();

private:
    Ui::EditAccessFilter *ui;
    static EditAccessFilter *s_EditAccessFilter;

    int m_index;
    QString m_address;
    QList<access_list> m_filterList;
};

#endif // EDITACCESSFILTER_H
