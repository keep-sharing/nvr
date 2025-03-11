#include "LivePage.h"
#include "ui_LivePage.h"
#include "MsDevice.h"
#include <QDesktopWidget>
#include <QtDebug>

LivePage *LivePage::s_livePage = nullptr;
LivePage *LivePage::s_mainPage = nullptr;
LivePage *LivePage::s_subPage = nullptr;

LivePage::Mode LivePage::s_mode = LivePage::ModeAuto;

LivePage::LivePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LivePage)
{
    ui->setupUi(this);
    s_livePage = this;

    //
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    m_limitTimer = new QTimer(this);
    m_limitTimer->setSingleShot(true);
    m_limitTimer->setInterval(1000);
}

LivePage::~LivePage()
{
    s_livePage = nullptr;
    delete ui;
}

LivePage *LivePage::instance()
{
    return s_livePage;
}

void LivePage::setMainPage(LivePage *page)
{
    s_mainPage = page;
}

LivePage *LivePage::mainPage()
{
    return s_mainPage;
}

void LivePage::setSubPage(LivePage *page)
{
    s_subPage = page;
}

LivePage *LivePage::subPage()
{
    return s_subPage;
}

void LivePage::initializeData()
{
    //
    struct display display_info = qMsNvr->displayInfo();
    switch (display_info.page_info) {
    case 0:
        setMode(LivePage::ModeAlwaysHide);
        break;
    case 1:
        setMode(LivePage::ModeAlwaysShow);
        break;
    case 2:
        setMode(LivePage::ModeAuto);
        break;
    }
}

void LivePage::setMode(LivePage::Mode mode)
{
    s_mode = mode;

    if (mainPage()) {
        mainPage()->setPageMode(mode);
    }

    if (subPage()) {
        subPage()->setPageMode(mode);
    }
}

LivePage::Mode LivePage::mode()
{
    return s_mode;
}

void LivePage::adjustPos(const QRect &parentRect)
{
    m_enterRect = QRect(parentRect.left() + parentRect.width() / 2 - width(),
                        parentRect.height() - height() * 3,
                        width() * 2,
                        height() * 2);

    move(parentRect.left() + parentRect.width() / 2 - width() / 2, parentRect.bottom() - height() * 2.5);
}

void LivePage::showOrHide(const QPoint &pos)
{
    if (s_mode != ModeAuto) {
        return;
    }

    if (m_timer->isActive()) {
        return;
    }

    if (m_enterRect.contains(pos)) {
        setVisible(true);
        raise();
    } else {
        setVisible(false);
    }
}

void LivePage::setPage(int page, int count)
{
    m_currentPage = page;
    m_pageCount = count;
    if (count == 0) {
        m_currentPage = 0;
        ui->label_page->setText(QString("0/0"));
        return;
    }

    ui->label_page->setText(QString("%1/%2").arg(m_currentPage + 1).arg(m_pageCount));
}

void LivePage::showPage(int msec)
{
    //qDebug() << "====LivePage::showPage====";
    //qDebug() << "----this:" << this;
    //qDebug() << "----sub:" << s_subPage;
    //qDebug() << "----mode:" << s_mode;
    if (s_mode != ModeAuto) {
        return;
    }

    show();
    raise();
    m_timer->start(msec);
}

void LivePage::showEvent(QShowEvent *)
{
    //qDebug() << "====LivePage::showEvent====";
    //qDebug() << "----this:" << this;
}

void LivePage::hideEvent(QHideEvent *)
{
    //qDebug() << "====LivePage::hideEvent====";
    //qDebug() << "----this:" << this;
}

void LivePage::setPageMode(LivePage::Mode mode)
{
    switch (mode) {
    case ModeAlwaysShow:
        setVisible(true);
        raise();
        break;
    case ModeAlwaysHide:
        setVisible(false);
        break;
    case ModeAuto:
        setVisible(false);
        break;
    }
}

void LivePage::onTimeout()
{
    //qDebug() << "====LivePage::onTimeout====";
    //qDebug() << "----this:" << this;
    if (s_mode != ModeAuto) {
        return;
    }

    hide();
}

void LivePage::on_toolButton_previousPage_clicked()
{
    if (m_limitTimer->isActive()) {
        return;
    }
    emit previousPage();
    m_limitTimer->start();
}

void LivePage::on_toolButton_nextPage_clicked()
{
    if (m_limitTimer->isActive()) {
        return;
    }
    emit nextPage();
    m_limitTimer->start();
}
