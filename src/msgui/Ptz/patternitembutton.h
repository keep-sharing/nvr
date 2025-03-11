#ifndef PATTERNITEMBUTTON_H
#define PATTERNITEMBUTTON_H

#include <QWidget>

namespace Ui {
class PatternItemButton;
}

class PatternItemButton : public QWidget
{
    Q_OBJECT

public:
    explicit PatternItemButton(int row, QWidget *parent = nullptr);
    ~PatternItemButton();

    void setButtonType(int type);

private:
    void setPlayButtonState(const QString &state);

signals:
    void buttonClicked(int row, int index);

private slots:
    void on_toolButton_play_clicked();
    void on_toolButton_record_clicked();
    void on_toolButton_delete_clicked();

private:
    Ui::PatternItemButton *ui;

    int m_currentRow = 0;
};

#endif // PATTERNITEMBUTTON_H
