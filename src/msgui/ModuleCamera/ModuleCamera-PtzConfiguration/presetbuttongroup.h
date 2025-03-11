#ifndef PRESETBUTTONGROUP_H
#define PRESETBUTTONGROUP_H

#include <QWidget>

namespace Ui {
class PresetButtonGroup;
}

class PresetButtonGroup : public QWidget
{
    Q_OBJECT

public:
    explicit PresetButtonGroup(int row, QWidget *parent = nullptr);
    ~PresetButtonGroup();

    void setButtonState(int state);

signals:
    void buttonClicked(int row, int buttonIndex);

private slots:
    void on_toolButton_play_clicked();
    void on_toolButton_save_clicked();
    void on_toolButton_delete_clicked();

private:
    Ui::PresetButtonGroup *ui;
    int m_row;
};

#endif // PRESETBUTTONGROUP_H
