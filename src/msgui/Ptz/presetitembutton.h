#ifndef PRESETITEMBUTTON_H
#define PRESETITEMBUTTON_H

#include <QWidget>

namespace Ui {
class PresetItemButton;
}

class PresetItemButton : public QWidget
{
    Q_OBJECT

public:
    explicit PresetItemButton(int row, QWidget *parent = 0);
    ~PresetItemButton();

    void setTheme(int theme);
    void setButtonType(int type);

signals:
    void buttonClicked(int row, int index);

private slots:
    void on_toolButton_save_clicked();
    void on_toolButton_delete_clicked();
    void on_toolButton_play_clicked();

private:
    Ui::PresetItemButton *ui;

    int m_currentRow = 0;
};

#endif // PRESETITEMBUTTON_H
