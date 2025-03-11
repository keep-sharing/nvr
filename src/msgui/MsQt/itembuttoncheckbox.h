#ifndef ITEMBUTTONCHECKBOX_H
#define ITEMBUTTONCHECKBOX_H

#include <QWidget>

namespace Ui {
class ItemButtonCheckBox;
}

class ItemButtonCheckBox : public QWidget
{
    Q_OBJECT

public:
    explicit ItemButtonCheckBox(int row, QWidget *parent = 0);
    ~ItemButtonCheckBox();

    void setChecked(bool checked);

signals:
    void checkBoxClicked(int row, bool checked);

private slots:
    void on_checkBox_clicked(bool checked);

private:
    Ui::ItemButtonCheckBox *ui;
    int m_row;
};

#endif // ITEMBUTTONCHECKBOX_H
