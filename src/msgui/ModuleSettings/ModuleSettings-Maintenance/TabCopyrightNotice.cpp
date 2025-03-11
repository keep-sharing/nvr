#include "TabCopyrightNotice.h"
#include "ui_TabCopyrightNotice.h"
#include "MsLanguage.h"

TabCopyrightNotice::TabCopyrightNotice(QWidget *parent) :
    AbstractSettingTab(parent),
    ui(new Ui::TabCopyrightNotice)
{
    ui->setupUi(this);
    ui->labelCopyrightTitle->setText(GET_TEXT("MAINTENANCE/179000", "Copyright Notice"));
    ui->labelCopyrightContent1->setText(GET_TEXT("MAINTENANCE/179001", "This software uses the Qt open-source framework, a cross-platform application and UI development framework provided by https://www.qt.io/ .") 
    + "\n" + GET_TEXT("MAINTENANCE/179002", "This software complies with the Qt https://doc.qt.io/qt-6/licensing.html . Licensed under https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html#SEC1 .")
    + "\n" + GET_TEXT("MAINTENANCE/179016", "In accordance with the license requirements, we provide the following information:")
    + "\n" + GET_TEXT("MAINTENANCE/179003", "● The source code for the Qt open-source version is available for download from https://download.qt.io/ .")
    + "\n" + GET_TEXT("MAINTENANCE/179004", "● Certain functionalities of this software are based on the core,gui,network and test function modules features provided by the Qt 4.8.7 framework. ")
    + "\n" + GET_TEXT("MAINTENANCE/179017", "The source code is available for download from [https://github.com/keep-sharing/qt-4.8.7 ]"));

    ui->labelAboutTile->setText(GET_TEXT("MAINTENANCE/179005", "About Qt"));
    ui->labelAboutContent1->setText(GET_TEXT("MAINTENANCE/179006", "This program uses Qt version 4.8.7.")
    + "\n" + GET_TEXT("MAINTENANCE/179007", "Qt is a C++ toolkit for cross-platform application development.")
    + "\n" + GET_TEXT("MAINTENANCE/179008", "Qt provides single-source portability across all major desktop operating systems. It is also available for embedded Linux and other embedded and mobile operating systems.")
    + "\n" + GET_TEXT("MAINTENANCE/179009", "Qt is available under multiple licensing options designed to accommodate the needs of our various users.")
    + "\n" + GET_TEXT("MAINTENANCE/179010", "Qt licensed under our commercial license agreement is appropriate for development of proprietary/commercial software where you do not want to share any source code ")
    + "\n" + GET_TEXT("MAINTENANCE/179018", "with third parties or otherwise cannot comply with the terms of GNU (L)GPL.")
    + "\n" + GET_TEXT("MAINTENANCE/179011", "Qt licensed under GNU (L)GPL is appropriate for the development of Qt applications provided you can comply with the terms and conditions of the respective licenses.")
    + "\n" + GET_TEXT("MAINTENANCE/179012", "Please see http://qt.io/licensing/  for an overview of Qt licensing.")
    + "\n" + GET_TEXT("MAINTENANCE/179013", "Copyright (C) 2022 The Qt Company Ltd and other contributors.")
    + "\n" + GET_TEXT("MAINTENANCE/179014", "Qt and the Qt logo are trademarks of The Qt Company Ltd.")
    + "\n" + GET_TEXT("MAINTENANCE/179015", "Qt is The Qt Company Ltd product developed as an open source project. See http://qt.io/  for more information."));

}

TabCopyrightNotice::~TabCopyrightNotice()
{
    delete ui;
}
