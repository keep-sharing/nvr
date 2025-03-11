#include "logout.h"
#include "ui_logout.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "mainwindow.h"
#include "msuser.h"
#include <QMouseEvent>
#include <QPainter>

extern "C" {
#include "log.h"
}

Logout::Logout(QWidget *parent)
    : BaseDialog(parent)
    , ui(new Ui::Logout)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    ui->label_title->setText(GET_TEXT("MENU/10012", "Logout"));

    ui->toolButton_logout->setText(GET_TEXT("MENU/10013", "Logout"));
    ui->toolButton_logout->setTextColor(QColor("#FFFFFF"));
    ui->toolButton_logout->setPixmap(QPixmap(":/common/common/logout.png"));
    ui->toolButton_reboot->setText(GET_TEXT("USER/74045", "Reboot"));
    ui->toolButton_reboot->setTextColor(QColor("#FFFFFF"));
    ui->toolButton_reboot->setPixmap(QPixmap(":/common/common/reboot.png"));
    ui->toolButton_shutdown->setText(GET_TEXT("MENU/10008", "Shutdown"));
    ui->toolButton_shutdown->setTextColor(QColor("#FFFFFF"));
    ui->toolButton_shutdown->setPixmap(QPixmap(":/common/common/shutdown.png"));

    showMaximized();
}

Logout::~Logout()
{
    delete ui;
}

void Logout::showEvent(QShowEvent *event)
{
    QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, ui->widget_content->size(), rect());
    ui->widget_content->setGeometry(rc);

    BaseDialog::showEvent(event);
}

void Logout::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(0, 0, 0, 60)));
    painter.drawRect(rect());
}

void Logout::mousePressEvent(QMouseEvent *event)
{
    if (ui->widget_content->geometry().contains(event->pos())) {

    } else {
        reject();
    }
    event->accept();
}

bool Logout::isMoveToCenter()
{
    return false;
}

bool Logout::isAddToVisibleList()
{
    return true;
}

void Logout::escapePressed()
{
    reject();
}

void Logout::on_toolButton_logout_clicked()
{
    ui->toolButton_logout->setAttribute(Qt::WA_UnderMouse, false);
    ui->toolButton_logout->update();
    const int &result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20033", "Do you want to logout?"), QString(), MessageBox::AlignCenter);
    if (result == MessageBox::Yes) {
        done(TypeLogout);
        //
        qMsNvr->closeTalkback();
        //
        qMsNvr->writeLog(SUB_OP_LOGOUT_LOCAL);
    }
}

void Logout::on_toolButton_reboot_clicked()
{
    ui->toolButton_reboot->setAttribute(Qt::WA_UnderMouse, false);
    ui->toolButton_reboot->update();
    if (!gMsUser.checkBasicPermission(PERM_MODE_LOGOUT, PERM_SHUTDOWN_NONE)) {
        ShowMessageBox(this, GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    //
    const int &result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20035", "Do you want to reboot?"), QString(), MessageBox::AlignCenter);
    if (result == MessageBox::Yes) {
        done(TypeReboot);
    }
}

void Logout::on_toolButton_shutdown_clicked()
{
    ui->toolButton_shutdown->setAttribute(Qt::WA_UnderMouse, false);
    ui->toolButton_shutdown->update();
    if (!gMsUser.checkBasicPermission(PERM_MODE_LOGOUT, PERM_SHUTDOWN_NONE)) {
        ShowMessageBox(this, GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    //
    if (qMsNvr->isSupportPhysicalShutdown()) {
        const int &result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20036", "Do you want to Shutdown?"), QString(), MessageBox::AlignCenter);
        if (result == MessageBox::Yes) {
            done(TypeShutdown);
        }
    } else {
        const int &result = MessageBox::question(this, GET_TEXT("SYSTEMGENERAL/70072", "NVR will stop recording after confirming, continue?"), QString(), MessageBox::AlignCenter);
        if (result == MessageBox::Yes) {
            done(TypeShutdown);
        }
    }
}
