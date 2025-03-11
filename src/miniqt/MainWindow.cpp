#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "VapiLayout.h"
#include <QPainter>
#include <QResizeEvent>
#include <QtDebug>

int callback_start_stream(int chan_id)
{
    CameraInfo *info = CameraInfo::fromChannel(chan_id);
    info->setDecodeEnable(true);
    // qDebug() << QString("channel: %1, ip: %2, decode: true").arg(info->channel()).arg(info->ip());

    return 0;
}

int callback_stop_stream(int chan_id)
{
    CameraInfo *info = CameraInfo::fromChannel(chan_id);
    info->setDecodeEnable(false);
    // qDebug() << QString("channel: %1, ip: %2, decode: false").arg(info->channel()).arg(info->ip());

    return 0;
}

void reco_cli_output(char *format, ...)
{
    Q_UNUSED(format)
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    //
    m_bottomBar = new BottomBar(this);
    connect(m_bottomBar, SIGNAL(layoutChanged(int,int)), this, SLOT(onLayoutChanged(int,int)));
    connect(m_bottomBar, SIGNAL(cameraManagement()), this, SLOT(onCameraManagement()));

    //
    reco_rtsp_init();

    struct reco_rtsp_server_conf conf;
    memset(&conf, 0, sizeof(conf));
    conf.server_port = 554;
    reco_rtsp_server_set(&conf);

    //
    QMetaObject::invokeMethod(this, "onLayoutChanged", Qt::QueuedConnection, Q_ARG(int, 2), Q_ARG(int, 2));

    //
    showMaximized();

    m_cameraManagement = new CameraManagement(this);
}

MainWindow::~MainWindow()
{
    vapi_uninit();
    reco_rtsp_uninit();

    delete ui;
}

int MainWindow::vapiInitialize(SCREEN_RES_E res)
{
    VAPI_ATTR_S stVapiAttr;
    memset(&stVapiAttr, 0, sizeof(VAPI_ATTR_S));
    stVapiAttr.enRes[SCREEN_MAIN]              = res;
    stVapiAttr.enRes[SCREEN_SUB]               = res;
    stVapiAttr.stCallback.start_stream_cb      = callback_start_stream;
    stVapiAttr.stCallback.stop_stream_cb       = callback_stop_stream;
    stVapiAttr.stCallback.update_chn_cb        = NULL;
    stVapiAttr.stCallback.update_screen_res_cb = NULL;
    stVapiAttr.isHotplug                       = 0;
    stVapiAttr.isogeny                         = 1;
    stVapiAttr.isDualScreen                    = 0;
    return vapi_init(&stVapiAttr);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (m_bottomBar) {
        m_bottomBar->resize(width(), 60);
        m_bottomBar->move(0, height() - m_bottomBar->height());
    }
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QFont f = painter.font();
    f.setPixelSize(20);
    painter.setFont(f);

    painter.setPen(Qt::red);

    int screenWidth, screenHeight;
    vapi_get_screen_res(SCREEN_MAIN, &screenWidth, &screenHeight);

    painter.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, QString("monitor resolution: %1 x %2").arg(screenWidth).arg(screenHeight));
}

void MainWindow::onLayoutChanged(int row, int column)
{
    ui->videoLayout->setVideoLayout(row, column);
    gVapiLayout.vapiSetLayout(row, column);
}

void MainWindow::onCameraManagement()
{
    m_cameraManagement->exec();
}
