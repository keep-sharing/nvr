#ifndef ITEMSWITCH_H
#define ITEMSWITCH_H

#include <QWidget>

namespace Ui {
class ItemSwitch;
}

class ItemSwitch : public QWidget
{
    Q_OBJECT

public:
    explicit ItemSwitch(int row, int column, bool checked, QWidget *parent = nullptr);
    ~ItemSwitch();

    void setChecked(bool checked);
    bool isChecked() const;

private slots:
    void on_toolButton_clicked(bool checked);

signals:
    void switchChanged(int row, int column, bool checked);

private:
    Ui::ItemSwitch *ui;

    int m_row = 0;
    int m_column = 0;
};

#endif // ITEMSWITCH_H
