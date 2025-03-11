#ifndef FISHEYEBAR_H
#define FISHEYEBAR_H

#include <QWidget>

namespace Ui {
class FisheyeBar;
}

class FisheyeBar : public QWidget
{
    Q_OBJECT

public:
    enum ButtonMode
    {
        Mode_PTZ,
        Mode_Close
    };

    explicit FisheyeBar(QWidget *parent = nullptr);
    ~FisheyeBar();

signals:
    void buttonClicked(int mode);

private slots:
    void on_toolButton_ptz_clicked();
    void on_toolButton_close_clicked();

private:
    Ui::FisheyeBar *ui;
};

#endif // FISHEYEBAR_H
