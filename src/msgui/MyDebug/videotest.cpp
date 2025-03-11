#include "videotest.h"
#include "ui_videotest.h"
#include "centralmessage.h"
#include "SubControl.h"
#include "mainwindow.h"
#include "settingcontent.h"

VideoTest::VideoTest(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::VideoTest)
{
    ui->setupUi(this);
    setTitleWidget(ui->label_title);
    setWindowModality(Qt::NonModal);
}

VideoTest::~VideoTest()
{
    delete ui;
}

void VideoTest::on_pushButton_play_clicked()
{
    req_pipmode_s pipmode;
    memset(&pipmode, 0, sizeof(pipmode));

    pipmode.enState = STATE_ENTER;
    pipmode.enScreen = SubControl::instance()->currentScreen();
    pipmode.enMode = DSP_MODE_LIVE;

    pipmode.stZone.enMode = ZONE_MODE_USER;
    pipmode.stZone.x = ui->spinBox_x->value();
    pipmode.stZone.y = ui->spinBox_y->value();
    pipmode.stZone.w = ui->spinBox_width->value();
    pipmode.stZone.h = ui->spinBox_height->value();

    pipmode.stDevInfo.devID = ui->spinBox_channel->value();
    pipmode.stDevInfo.voutID = 0;
    pipmode.stDevInfo.stFormat[STREAM_MAIN].enType = ENC_TYPE_H264;
    pipmode.stDevInfo.stFormat[STREAM_MAIN].fps = 25;
    pipmode.stDevInfo.stFormat[STREAM_MAIN].width = 1280;
    pipmode.stDevInfo.stFormat[STREAM_MAIN].height = 720;
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_PIPMODE, (void *)&pipmode, sizeof(pipmode));

}

void VideoTest::on_pushButton_stop_clicked()
{
    req_pipmode_s pipmode;
    memset(&pipmode, 0, sizeof(pipmode));

    pipmode.enState = STATE_EXIT;
    pipmode.enScreen = SubControl::instance()->currentScreen();
    pipmode.enMode = DSP_MODE_LIVE;

    pipmode.stZone.enMode = ZONE_MODE_USER;
    pipmode.stZone.x = ui->spinBox_x->value();
    pipmode.stZone.y = ui->spinBox_y->value();
    pipmode.stZone.w = ui->spinBox_width->value();
    pipmode.stZone.h = ui->spinBox_height->value();

    pipmode.stDevInfo.devID = ui->spinBox_channel->value();
    pipmode.stDevInfo.voutID = 0;
    pipmode.stDevInfo.stFormat[STREAM_MAIN].enType = ENC_TYPE_H264;
    pipmode.stDevInfo.stFormat[STREAM_MAIN].fps = 25;
    pipmode.stDevInfo.stFormat[STREAM_MAIN].width = 1280;
    pipmode.stDevInfo.stFormat[STREAM_MAIN].height = 720;
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_PIPMODE, (void *)&pipmode, sizeof(pipmode));
}

void VideoTest::on_toolButton_close_clicked()
{
    close();

    if (!SettingContent::instance()->isVisible())
    {
        MainWindow::instance()->showLiveView();
    }
}

void VideoTest::on_pushButton_hideLiveView_clicked()
{
    MainWindow::instance()->hideLiveView();
}
