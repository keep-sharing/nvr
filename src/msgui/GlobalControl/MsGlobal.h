#ifndef MSGLOBAL_H
#define MSGLOBAL_H

#include <QWidget>
#include <QString>

class SplashDialog;
class MessageBox;

extern SplashDialog *g_splashMain;
extern SplashDialog *g_splashSub;
extern MessageBox *g_messageBox;
extern bool g_isFirstLogin;
extern bool g_isReadyQuit;

QString VapiWinIdString(int winid);
//
QString FisheyeTransferModeString(int mode);
QString FisheyeInstallModeString(int mode);
QString FisheyeDisplayModeString(int mode);
QString FisheyeMinoridString(int minorid);
//
QString FisheyeInstallModeString_SoftDewarp(int mode);
QString FisheyeDisplayModeString_SoftDewarp(int mode);

//显示在屏幕中间
void ShowMessageBox(const QString &text);
//显示在widget中间
void ShowMessageBox(QWidget *widget, const QString &text);
//
void ExecMessageBox(const QString &text);
void ExecMessageBox(QWidget *widget, const QString &text);
//
int ExecQuestionBox(const QString &text);
int ExecQuestionBox(QWidget *widget, const QString &text);

#endif // MSGLOBAL_H
