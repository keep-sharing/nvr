#include "MsGlobal.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include "mainwindow.h"
#include "SubControl.h"

extern "C" {
#include "msg.h"
}

SplashDialog *g_splashMain = nullptr;
SplashDialog *g_splashSub = nullptr;
MessageBox *g_messageBox = nullptr;
bool g_isFirstLogin = true;
bool g_isReadyQuit = false;

QString VapiWinIdString(int winid)
{
    if (winid < 0) {
        qMsWarning() << "invalid winid:" << winid;
        return QString("%1").arg(winid);
    }
    int screen = (winid >> 16);
    int sid = (winid & 0xFF);
    QString text = QString("VAPI_WIN_ID(0x%1, screen: %2, sid: %3)").arg(winid, 0, 16).arg(screen).arg(sid);
    return text;
}

QString FisheyeInstallModeString_SoftDewarp(int mode)
{
    QString text;
#ifdef MS_FISHEYE_SOFT_DEWARP
    switch (mode) {
    case Ceilling:
        text = "Ceilling";
        break;
    case flat:
        text = "flat";
        break;
    case wall:
        text = "wall";
        break;
    default:
        text = "Unknow";
        break;
    }
#endif
    return QString("%1(%2)").arg(text).arg(mode);
}

QString FisheyeDisplayModeString_SoftDewarp(int mode)
{
    QString text;
#ifdef MS_FISHEYE_SOFT_DEWARP
    switch (mode) {
    case FISH_MODE_NONE:
        text = "FISH_MODE_NONE";
        break;
    case FISH_MODE_1O:
        text = "FISH_MODE_1O";
        break;
    case FISH_MODE_1P:
        text = "FISH_MODE_1P";
        break;
    case FISH_MODE_1W:
        text = "FISH_MODE_1W";
        break;
    case FISH_MODE_2P:
        text = "FISH_MODE_2P";
        break;
    case FISH_MODE_1P1R:
        text = "FISH_MODE_1P1R";
        break;
    case FISH_MODE_1W1R:
        text = "FISH_MODE_1W1R";
        break;
    case FISH_MODE_1O3R:
        text = "FISH_MODE_1O3R";
        break;
    case FISH_MODE_1P4R:
        text = "FISH_MODE_1P4R";
        break;
    case FISH_MODE_1W4R:
        text = "FISH_MODE_1W4R";
        break;
    case FISH_MODE_1P6R:
        text = "FISH_MODE_1P6R";
        break;
    case FISH_MODE_4R:
        text = "FISH_MODE_4R";
        break;
    case FISH_MODE_1O8R:
        text = "FISH_MODE_1O8R";
        break;
    case FISH_MODE_1O1R1W2P1P:
        text = "FISH_MODE_1O1R1W2P1P";
        break;
    case FISH_MODE_1O1R:
        text = "FISH_MODE_1O1R";
        break;
    default:
        text = "Unknow";
        break;
    }
#endif
    return QString("%1(%2)").arg(text).arg(mode);
}

void ShowMessageBox(const QString &text)
{
    ShowMessageBox(nullptr, text);
}

void ShowMessageBox(QWidget *widget, const QString &text)
{
    if (!g_messageBox) {
        g_messageBox = new MessageBox(MainWindow::instance());
    }
    if (widget) {
        QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, g_messageBox->size(), QRect(widget->mapToGlobal(QPoint(0, 0)), widget->size()));
        g_messageBox->move(rc.topLeft());
    } else {
        QRect screenRc = SubControl::instance()->logicalMainScreenGeometry();
        QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, g_messageBox->size(), screenRc);
        g_messageBox->move(rc.topLeft());
    }
    g_messageBox->showInformation(text);
    g_messageBox->show();
}

void ExecMessageBox(const QString &text)
{
    ExecMessageBox(nullptr, text);
}

void ExecMessageBox(QWidget *widget, const QString &text)
{
    if (!g_messageBox) {
        g_messageBox = new MessageBox(MainWindow::instance());
    }
    if (widget) {
        QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, g_messageBox->size(), QRect(widget->mapToGlobal(QPoint(0, 0)), widget->size()));
        g_messageBox->move(rc.topLeft());
    } else {
        QRect screenRc = SubControl::instance()->logicalMainScreenGeometry();
        QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, g_messageBox->size(), screenRc);
        g_messageBox->move(rc.topLeft());
    }
    g_messageBox->showInformation(text);
    g_messageBox->exec();
}

int ExecQuestionBox(const QString &text)
{
    return ExecQuestionBox(nullptr, text);
}

int ExecQuestionBox(QWidget *widget, const QString &text)
{
    if (!g_messageBox) {
        g_messageBox = new MessageBox(MainWindow::instance());
    }
    if (widget) {
        QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, g_messageBox->size(), QRect(widget->mapToGlobal(QPoint(0, 0)), widget->size()));
        g_messageBox->move(rc.topLeft());
    } else {
        QRect screenRc = SubControl::instance()->logicalMainScreenGeometry();
        QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, g_messageBox->size(), screenRc);
        g_messageBox->move(rc.topLeft());
    }
    g_messageBox->showQuestion(text, QString());
    return g_messageBox->exec();
}

QString FisheyeTransferModeString(int mode)
{
    QString text;
    switch (mode) {
    case 0:
        text = "Multi-Channel";
        break;
    case 1:
        text = "Bundle-Stream";
        break;
    default:
        text = "Unknow";
        break;
    }
    return QString("%1(%2)").arg(text).arg(mode);
}

QString FisheyeInstallModeString(int mode)
{
    QString text;
    switch (mode) {
    case MsQt::FISHEYE_INSTALL_CEILING:
        text = "FISHEYE_INSTALL_CEILING";
        break;
    case MsQt::FISHEYE_INSTALL_WALL:
        text = "FISHEYE_INSTALL_WALL";
        break;
    case MsQt::FISHEYE_INSTALL_FLAT:
        text = "FISHEYE_INSTALL_FLAT";
        break;
    default:
        text = "FISHEYE_INSTALL_UNKNOW";
        break;
    }
    return QString("%1(%2)").arg(text).arg(mode);
}

QString FisheyeDisplayModeString(int mode)
{
    QString text;
    switch (mode) {
    case MsQt::FISHEYE_DISPLAY_1O:
        text = "FISHEYE_DISPLAY_1O";
        break;
    case MsQt::FISHEYE_DISPLAY_1P:
        text = "FISHEYE_DISPLAY_1P";
        break;
    case MsQt::FISHEYE_DISPLAY_2P:
        text = "FISHEYE_DISPLAY_2P";
        break;
    case MsQt::FISHEYE_DISPLAY_4R:
        text = "FISHEYE_DISPLAY_4R";
        break;
    case MsQt::FISHEYE_DISPLAY_1O3R:
        text = "FISHEYE_DISPLAY_1O3R";
        break;
    case MsQt::FISHEYE_DISPLAY_1P3R:
        text = "FISHEYE_DISPLAY_1P3R";
        break;
    case MsQt::FISHEYE_DISPLAY_1O1P3R:
        text = "FISHEYE_DISPLAY_1O1P3R";
        break;
    default:
        text = "FISHEYE_DISPLAY_UNKNOW";
        break;
    }
    return QString("%1(%2)").arg(text).arg(mode);
}

QString FisheyeMinoridString(int minorid)
{
    QString text;
    if (minorid == 0) {
        text = QString("Minorid(0)");
    } 
    return text;
}
