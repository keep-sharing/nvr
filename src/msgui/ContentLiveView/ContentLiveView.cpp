#include "ContentLiveView.h"
#include <QFile>
#include "mainwindow.h"
#include "LiveView.h"
#include "LiveViewSub.h"
#include "SubControl.h"

ContentLiveView *ContentLiveView::self = nullptr;

ContentLiveView::ContentLiveView(QWidget *parent) :
    QWidget(parent)
{
    //加载样式表
    QFile file(":/style/style/liveviewstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QString strStyle = file.readAll();
        setStyleSheet(strStyle);
    }
    else
    {
        qWarning() << QString("ContentLiveView load style failed.");
    }
    file.close();
}

ContentLiveView::~ContentLiveView()
{

}

ContentLiveView *ContentLiveView::instance()
{
    if (!self)
    {
        self = new ContentLiveView(MainWindow::instance());
    }
    return self;
}

void ContentLiveView::setGeometry(const QRect &rc)
{
    QWidget::setGeometry(rc);

    //
    LiveView::instance()->setGeometry(SubControl::instance()->logicalMainScreenGeometry());
    if (LiveViewSub::instance())
    {
        LiveViewSub::instance()->setGeometry(SubControl::instance()->logicalSubScreenGeometry());
    }
}
