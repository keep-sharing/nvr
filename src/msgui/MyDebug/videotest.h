#ifndef VIDEOTEST_H
#define VIDEOTEST_H

#include "BaseShadowDialog.h"

namespace Ui {
class VideoTest;
}

class VideoTest : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit VideoTest(QWidget *parent = nullptr);
    ~VideoTest();

private slots:
    void on_pushButton_play_clicked();

    void on_pushButton_stop_clicked();

    void on_toolButton_close_clicked();

    void on_pushButton_hideLiveView_clicked();

private:
    Ui::VideoTest *ui;
};

#endif // VIDEOTEST_H
