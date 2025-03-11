#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include <QWidget>

namespace Ui {
class BottomBar;
}

class BottomBar : public QWidget
{
    Q_OBJECT

public:
    explicit BottomBar(QWidget *parent = nullptr);
    ~BottomBar();

protected:
    void paintEvent(QPaintEvent *) override;

signals:
    void layoutChanged(int row, int column);
    void cameraManagement();

private slots:
    void on_toolButton_1_clicked();
    void on_toolButton_4_clicked();
    void on_toolButton_9_clicked();
    void on_toolButton_12_clicked();
    void on_toolButton_16_clicked();
    void on_toolButton_25_clicked();
    void on_toolButton_36_clicked();

    void on_pushButtonCameraManagement_clicked();

private:
    Ui::BottomBar *ui;
};

#endif // BOTTOMBAR_H
