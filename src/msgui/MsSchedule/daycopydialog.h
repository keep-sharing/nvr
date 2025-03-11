#ifndef DAYCOPYDIALOG_H
#define DAYCOPYDIALOG_H

#include "BaseShadowDialog.h"

class QCheckBox;

namespace Ui {
class DayCopyDialog;
}

class DayCopyDialog : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit DayCopyDialog(QWidget *parent = nullptr);
    ~DayCopyDialog();

    void setCurrentDay(int day);
    QList<int> dayList() const;
    void setHolidayVisible(bool visible);

private slots:
    void onLanguageChanged();
    void onCheckBoxClicked();
    void on_checkBox_all_clicked(bool checked);
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

  private:
    Ui::DayCopyDialog *ui;

    QList<QCheckBox *> m_checkBoxList;
};

#endif // DAYCOPYDIALOG_H
