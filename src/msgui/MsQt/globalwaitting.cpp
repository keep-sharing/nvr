#include "globalwaitting.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>
#include <QtDebug>
#include "mainwindow.h"
#include "SubControl.h"

GlobalWaitting *GlobalWaitting::s_gWaitting = nullptr;

GlobalWaitting::GlobalWaitting(QWidget *parent) :
    MsWaitting(parent)
{
    setCustomPos(true);
}

GlobalWaitting *GlobalWaitting::instance()
{
    if (!s_gWaitting)
    {
        s_gWaitting = new GlobalWaitting(MainWindow::instance());
    }
    return s_gWaitting;
}

void GlobalWaitting::showWait()
{
    GlobalWaitting *waitting = GlobalWaitting::instance();
    waitting->moveToScreenCenter();
    waitting->show();
}

void GlobalWaitting::showWait(const QRect &rc)
{
    GlobalWaitting *waitting = GlobalWaitting::instance();

    if (!rc.isNull())
    {
        waitting->move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, waitting->size(), rc).topLeft());
    }
    else
    {
        waitting->moveToScreenCenter();
    }
    waitting->show();
}

void GlobalWaitting::showWait(QWidget *widget)
{
    GlobalWaitting *waitting = GlobalWaitting::instance();

    if (widget)
    {
        QRect widgetRect(widget->mapToGlobal(QPoint(0, 0)), widget->size());
        waitting->move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, waitting->size(), widgetRect).topLeft());
    }
    else
    {
        waitting->moveToScreenCenter();
    }
    waitting->show();
}

int GlobalWaitting::execWait(const QRect &rc)
{
    GlobalWaitting *waitting = GlobalWaitting::instance();

    if (!rc.isNull())
    {
        waitting->move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, waitting->size(), rc).topLeft());
    }
    else
    {
        waitting->moveToScreenCenter();
    }
    return waitting->exec();
}

int GlobalWaitting::execWait(QWidget *widget)
{
    GlobalWaitting *waitting = GlobalWaitting::instance();

    if (widget)
    {
        QRect widgetRect(widget->mapToGlobal(QPoint(0, 0)), widget->size());
        waitting->move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, waitting->size(), widgetRect).topLeft());
    }
    else
    {
        waitting->moveToScreenCenter();
    }
    return waitting->exec();
}

void GlobalWaitting::closeWait()
{
    GlobalWaitting *waitting = GlobalWaitting::instance();

    if (waitting)
    {
        waitting->close();
    }
}

void GlobalWaitting::moveToScreenCenter()
{
    QRect rc = SubControl::instance()->logicalMainScreenGeometry();
    move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), rc).topLeft());
}
