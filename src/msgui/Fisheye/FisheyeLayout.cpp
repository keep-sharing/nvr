#include "FisheyeLayout.h"
#include "ui_FisheyeLayout.h"
#include <QMouseEvent>
#include <QPainter>
#include "MyDebug.h"
#include "MsDevice.h"
#include "FisheyeControl.h"

FisheyeLayout *FisheyeLayout::s_fisheyeLayout = nullptr;
FisheyeLayout::FisheyeLayout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FisheyeLayout)
{
    ui->setupUi(this);
    s_fisheyeLayout = this;

    m_fisheyeBar = new FisheyeBar(this);
    m_fisheyeBar->hide();
    connect(m_fisheyeBar, SIGNAL(buttonClicked(int)), this, SLOT(onFisheyeButtonClicked(int)));
}

FisheyeLayout::~FisheyeLayout()
{
    s_fisheyeLayout = nullptr;
    delete ui;
}

FisheyeLayout *FisheyeLayout::instance()
{
    return s_fisheyeLayout;
}

void FisheyeLayout::setLayoutMode(int mode)
{
    m_layoutMode = mode;
    update();
}

int FisheyeLayout::layoutMode()
{
    return m_layoutMode;
}

QRect FisheyeLayout::selectedRect()
{
    switch (m_layoutMode) {
    case MsQt::FISHEYE_DISPLAY_1P:
    case MsQt::FISHEYE_DISPLAY_2P:
        return rect();
        break;
    default:
        return m_selectedRect;
        break;
    }
}

QRect FisheyeLayout::selectedGlobalRect()
{
    QRect rc = selectedRect();
    if (rc.isValid())
    {
        rc.moveTopLeft(mapToGlobal(QPoint(0, 0)) + rc.topLeft());
    }
    return rc;
}

void FisheyeLayout::setFisheyeBarVisible(bool visible)
{
    m_showFisheyeBar = visible;
    if (!m_showFisheyeBar)
    {
        m_selectedRect.setWidth(0);
    }
    update();
}

int FisheyeLayout::currentFisheyeStream()
{
    return m_selectedStream;
}

FisheyeBar *FisheyeLayout::fisheyeBar()
{
    return m_fisheyeBar;
}

void FisheyeLayout::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    const QRect &rc = rect();
    const QPoint &p = event->pos();
    if (p.x() < width() / 2)
    {
        m_selectedRect.setLeft(rc.left() + 1);
        m_selectedRect.setRight(width() / 2);

        m_selectedStream = 0;
    }
    else
    {
        m_selectedRect.setLeft(width() / 2);
        m_selectedRect.setRight(rc.right() - 1);

        m_selectedStream = 1;
    }
    if (p.y() < height() / 2)
    {
        m_selectedRect.setTop(rc.top() + 1);
        m_selectedRect.setBottom(height() / 2);
    }
    else
    {
        m_selectedRect.setTop(height() / 2);
        m_selectedRect.setBottom(rc.bottom() - 1);

        if (m_selectedStream == 0)
        {
            m_selectedStream = 2;
        }
        else if (m_selectedStream == 1)
        {
            m_selectedStream = 3;
        }
    }
    m_showFisheyeBar = true;
    update();
}

void FisheyeLayout::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    switch (m_layoutMode) {
    case MsQt::FISHEYE_DISPLAY_1O:
    {
        m_fisheyeBar->hide();
        break;
    }
    case MsQt::FISHEYE_DISPLAY_1P:
    case MsQt::FISHEYE_DISPLAY_2P:
    {
        if (m_showFisheyeBar)
        {
            QPoint point(width() / 2 - m_fisheyeBar->width() / 2, 2);
            m_fisheyeBar->move(point);
            m_fisheyeBar->show();
        }
        break;
    }
    case MsQt::FISHEYE_DISPLAY_1O3R:
    {
        drawSelectedRect(&painter);
        if (m_selectedStream == 0)
        {
            m_fisheyeBar->hide();
        }
        else
        {
            if (m_selectedRect.width() != 0)
            {
                if (m_showFisheyeBar)
                {
                    QPoint point;
                    point.setX(m_selectedRect.left() + m_selectedRect.width() / 2 - m_fisheyeBar->width() / 2);
                    point.setY(m_selectedRect.top() + 2);
                    m_fisheyeBar->move(point);
                    m_fisheyeBar->show();
                }
            }
            else
            {
                m_fisheyeBar->hide();
            }
        }
        break;
    }
    case MsQt::FISHEYE_DISPLAY_4R:
    case MsQt::FISHEYE_DISPLAY_1P3R:
    {
        drawSelectedRect(&painter);
        if (m_selectedRect.width() != 0)
        {
            if (m_showFisheyeBar)
            {
                QPoint point;
                point.setX(m_selectedRect.left() + m_selectedRect.width() / 2 - m_fisheyeBar->width() / 2);
                point.setY(m_selectedRect.top() + 2);
                m_fisheyeBar->move(point);
                m_fisheyeBar->show();
            }
        }
        else
        {
            m_fisheyeBar->hide();
        }
        break;
    }
    default:
        break;
    }
}

void FisheyeLayout::drawSelectedRect(QPainter *painter)
{
    if (m_selectedRect.width() == 0)
    {
        return;
    }
    painter->save();
    painter->setPen(QPen(QColor("#FFFFFF"), 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(m_selectedRect);
    painter->restore();
}

void FisheyeLayout::onFisheyeButtonClicked(int mode)
{
    switch (mode) {
    case FisheyeBar::Mode_PTZ:
        m_fisheyeBar->hide();
        m_showFisheyeBar = false;
        break;
    case FisheyeBar::Mode_Close:
        m_fisheyeBar->hide();
        m_selectedRect.setWidth(0);
        m_showFisheyeBar = false;
        update();
        break;
    }
}
