#ifndef PATROLITEMBUTTON_H
#define PATROLITEMBUTTON_H

#include <QWidget>

namespace Ui {
class PatrolItemButton;
}

class PatrolItemButton : public QWidget
{
    Q_OBJECT

public:
    explicit PatrolItemButton(int row, QWidget *parent = nullptr);
    ~PatrolItemButton();

    void setButtonType(int type);

private:
    void setPlayButtonState(const QString &state);

signals:
    void buttonClicked(int row, int index);

private slots:
    void on_toolButton_play_clicked();
    void on_toolButton_setting_clicked();
    void on_toolButton_delete_clicked();

private:
    Ui::PatrolItemButton *ui;

    int m_currentRow = 0;
};

#endif // PATROLITEMBUTTON_H
