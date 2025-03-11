#include "VideoContainer.h"
#include "ui_VideoContainer.h"
#include "LiveVideo.h"
#include "mainwindow.h"
#include "MsDevice.h"
#include <QMouseEvent>

VideoContainer::VideoContainer(int index, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoContainer)
    , m_index(index)
{
    ui->setupUi(this);

    ui->toolButtonLayout->hide();

    connect(qMsNvr, SIGNAL(displayColorChanged(QColor)), this, SLOT(onDisplayColorChanged(QColor)));
}

VideoContainer::~VideoContainer()
{
    delete ui;
}

int VideoContainer::index() const
{
    return m_index;
}

void VideoContainer::setGlobalIndex(int index)
{
    m_globalIndex = index;
}

int VideoContainer::globalIndex() const
{
    return m_globalIndex;
}

void VideoContainer::setScreen(int screen)
{
    m_screen = screen;
}

int VideoContainer::screen() const
{
    return m_screen;
}

void VideoContainer::setVideo(LiveVideo *video)
{
    if (m_video) {
        clearVideo();
    }

    m_video = video;
    m_video->setParent(this);
    m_video->setGeometry(rect());
    m_video->setContainer(this);
    m_video->show();
}

void VideoContainer::clearVideo()
{
    if (m_video) {
        m_video->clearContainer();
        m_video->hide();
    }
    m_video = nullptr;
}

LiveVideo *VideoContainer::video() const
{
    return m_video;
}

QRect VideoContainer::globalGeometry() const
{
    return QRect(mapToGlobal(QPoint(0, 0)), size());
}

int VideoContainer::vapiWinId() const
{
    if (m_screen < 0) {
        return -1;
    }
    if (m_index < 0) {
        return -1;
    }
    return VAPI_WIN_ID(m_screen, m_index);
}

void VideoContainer::showLayoutButton()
{
    ui->toolButtonLayout->show();
}

void VideoContainer::hideLayoutButton()
{
    ui->toolButtonLayout->hide();
}

void VideoContainer::paintEvent(QPaintEvent *)
{
    if (LiveVideo::s_showBorder) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(LiveVideo::s_infoColor, 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(rect());

        //painter.drawText(0, 100, VapiWinIdString(vapiWinId()));
    }
}

void VideoContainer::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void VideoContainer::resizeEvent(QResizeEvent *event)
{
    if (m_video) {
        m_video->setGeometry(rect());
    }

    QRect rc1 = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, ui->toolButtonLayout->size(), rect());
    ui->toolButtonLayout->move(rc1.topLeft());

    QWidget::resizeEvent(event);
}

void VideoContainer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int channel = -1;
        if (m_video) {
            channel = m_video->channel();
        }

        if (globalIndex() < qMsNvr->maxChannel()) {
            emit clicked(channel);
        }
    }

    QWidget::mousePressEvent(event);
}

void VideoContainer::onDisplayColorChanged(const QColor &color)
{
    Q_UNUSED(color)

    update();
}

void VideoContainer::on_toolButtonLayout_clicked()
{
    QTimer::singleShot(0, MainWindow::instance(), SLOT(enterMenuLayout()));
}
