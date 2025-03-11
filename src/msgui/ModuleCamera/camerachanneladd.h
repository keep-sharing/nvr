#ifndef CAMERACHANNELADD_H
#define CAMERACHANNELADD_H

#include "BaseShadowDialog.h"

class QCheckBox;

namespace Ui {
class CameraChannelAdd;
}

class CameraChannelAdd : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit CameraChannelAdd(QWidget *parent = nullptr);
    ~CameraChannelAdd();

    int execAdd(const QMap<int, int> &map, int count);
    QList<int> addList() const;

private slots:
    void onLanguageChanged();

    void onCheckboxClicked();
    void on_checkBox_all_clicked(bool checked);

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::CameraChannelAdd *ui;

    QList<QCheckBox *> m_allCheckBox;
    QList<QCheckBox *> m_realCheckBoxList;
    QList<int> m_addList;
};

#endif // CAMERACHANNELADD_H
