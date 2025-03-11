#ifndef PATTERNBUTTONGROUP_H
#define PATTERNBUTTONGROUP_H

#include <QWidget>

namespace Ui {
class PatternButtonGroup;
}

class PatternButtonGroup : public QWidget
{
    Q_OBJECT

public:
    explicit PatternButtonGroup(int row, QWidget *parent = nullptr);
    ~PatternButtonGroup();

    void setButtonState(int state);

private:
    void setPlayButtonState(const QString &state);

signals:
    void buttonClicked(int row, int buttonIndex);

private slots:
    void on_toolButton_play_clicked();
    void on_toolButton_record_clicked();
    void on_toolButton_delete_clicked();

private:
    Ui::PatternButtonGroup *ui;
    int m_row;
};

#endif // PATTERNBUTTONGROUP_H
