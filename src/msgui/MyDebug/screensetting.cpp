#include "screensetting.h"
#include "ui_screensetting.h"
#include <QSettings>
#include <QDesktopWidget>
#include <QFile>
#include <QWSServer>
#include <QScreen>
#include <QWSDisplay>
#include <QtDebug>

ScreenSetting::ScreenSetting(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::ScreenSetting)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setWindowModality(Qt::NonModal);
    setTitleWidget(ui->label_title);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        setStyleSheet(file.readAll());
    }
    else
    {
        qWarning() << QString("SettingContent load style failed.");
    }
    file.close();
}

ScreenSetting::~ScreenSetting()
{
    delete ui;
}

void ScreenSetting::initializeData()
{
    QScreen *screen = QScreen::instance();
    const int screenCount = qApp->desktop()->numScreens();
    ui->textEdit_info->clear();
    ui->textEdit_info->append(QString("screen count: %1, ClassId: %2").arg(screenCount).arg(screen->classId()));
    for (int i = 0; i < screenCount; ++i)
    {
        const QRect &rc = qApp->desktop()->availableGeometry(i);
        ui->textEdit_info->append(QString("screen%1: x:%2, y: %3, width: %4, height: %5").arg(i).arg(rc.x()).arg(rc.y()).arg(rc.width()).arg(rc.height()));
    }

    QSettings setting("/opt/app/bin/setting.ini", QSettings::IniFormat);
    const QString &strScreenParameters = setting.value("screen").toString();
    ui->lineEdit_parameters->setText(strScreenParameters);

    //
    if (screenCount > 1)
    {
        if (!m_label)
        {
            m_label = new QLabel("Hello");
        }
        m_label->setGeometry(QApplication::desktop()->availableGeometry(1));
        m_label->showMaximized();
    }
}

void ScreenSetting::on_pushButton_set_clicked()
{
    const QString &strParameters = ui->lineEdit_parameters->text();
    QSettings setting("/opt/app/bin/setting.ini", QSettings::IniFormat);
    setting.setValue("screen", strParameters);

    if (m_label)
    {
        m_label->close();
        m_label->deleteLater();
        m_label = nullptr;
    }

    QWSServer *server = QWSServer::instance();
    QScreen *screen = QScreen::instance();

//    tde_fb_scale(currentScreen(), NO_EVENT);

    QWSServer::setCursorVisible(false);
    server->enablePainting(false);
    screen->shutdownDevice();
    screen->disconnect();

//    int fb = displaySpec.right(1).toInt();
//    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_WRITE_FRAMEBUFFER, (void*)&fb, sizeof(int));

    screen->connect(strParameters);
    screen->initDevice();
    server->enablePainting(true);
    server->refresh();
    QWSServer::setCursorVisible(true);

//    SCREEN_E screen_e = (fb == 1) ? SCREEN_MAIN : SCREEN_SUB;
//    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_CLEAR_BUFFER, (void *)&screen_e, sizeof(int));
//    qDebug() << QString("Clear %1.").arg((fb == 1) ? QString("MainScreen") : QString("SubScreen"));
}

void ScreenSetting::on_pushButton_default_clicked()
{
    ui->lineEdit_parameters->setText("multi: LinuxFb:/dev/fb0:0 LinuxFb:/dev/fb1:offset=1920,0:1 :0");
}

void ScreenSetting::on_pushButton_refresh_clicked()
{
    initializeData();
}

void ScreenSetting::on_toolButton_close_clicked()
{
    close();
}
