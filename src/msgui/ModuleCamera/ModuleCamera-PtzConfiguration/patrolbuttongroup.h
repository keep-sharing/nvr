#ifndef PATROLBUTTONGROUP_H
#define PATROLBUTTONGROUP_H

#include <QWidget>

namespace Ui {
class PatrolButtonGroup;
}

class PatrolButtonGroup : public QWidget
{
    Q_OBJECT

public:
    explicit PatrolButtonGroup(int row, QWidget *parent = nullptr);
    ~PatrolButtonGroup();

    void setButtonState(int state);

private:
    void setPlayButtonState(const QString &state);

signals:
    void buttonClicked(int row, int buttonIndex);

private slots:
    void on_toolButton_play_clicked();
    void on_toolButton_setting_clicked();
    void on_toolButton_delete_clicked();

private:
    Ui::PatrolButtonGroup *ui;
    int m_row;
};

#endif // PATROLBUTTONGROUP_H
