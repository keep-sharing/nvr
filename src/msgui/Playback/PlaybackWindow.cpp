#include "PlaybackWindow.h"
#include "ui_PlaybackWindow.h"
#include "BasePlayback.h"
#include "FilterEventPanel.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PlaybackData.h"
#include "PlaybackEventData.h"
#include "PlaybackList.h"
#include "PlaybackRealTimeThread.h"
#include "PlaybackSplit.h"
#include "PlaybackVideoBar.h"
#include "SmartSearchControl.h"
#include "SmartSpeedPanel.h"
#include "SubControl.h"
#include "centralmessage.h"
#include <QDesktopWidget>
#include <QFile>
#include <QMouseEvent>

#ifdef MS_FISHEYE_SOFT_DEWARP
#include "FisheyeDewarpControl.h"
#endif

extern "C" {

}

PlaybackWindow *PlaybackWindow::s_playback = nullptr;

PlaybackWindow::PlaybackWindow(QWidget *parent)
    : BaseWidget(parent)
{
    s_playback = this;
    //
    m_videoBar = new PlaybackVideoBar(this);
    m_videoBar->hide();

    ui = new Ui::Playback;
    ui->setupUi(this);

    //
    QFile file(":/style/style/playbackstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    } else {
        qWarning() << QString("Playback load style failed.");
    }
    file.close();

    //
    connect(ui->page_mode, SIGNAL(playbackModeChanged(MsPlaybackType)), this, SLOT(onPlaybackModeChanged(MsPlaybackType)));
    connect(ui->page_mode, SIGNAL(playbackModeChanged(MsPlaybackType)), ui->playbackBar, SLOT(onPlaybackModeChanged(MsPlaybackType)));

    //
    BasePlayback::s_thumbWidget = new ThumbWidget(this);
    BasePlayback::s_thumbWidget->hide();

    //
    PlaybackZoom *playbackZoom = new PlaybackZoom(this);
    playbackZoom->hide();
    connect(playbackZoom, SIGNAL(zoomStateChanged(int)), ui->playbackBar, SLOT(onZoomStateChanged(int)));

    //
    m_layoutSplit = new PlaybackLayoutSplit(this);
    m_layoutSplit->hide();

    //
    ui->stackedWidget_left->setCurrentIndex(0);
    ui->stackedWidget_right->setCurrentIndex(0);

    //
#ifdef MS_FISHEYE_SOFT_DEWARP
    BasePlayback::s_fisheyeControl = new FisheyeDewarpControl(this);
    BasePlayback::s_fisheyeControl->hide();
    connect(ui->playbackBar, SIGNAL(fisheyePanelButtonClicked(int, int)), BasePlayback::s_fisheyeControl, SLOT(onPlaybackFisheyePanelButtonClicked(int, int)));
    connect(BasePlayback::s_fisheyeControl, SIGNAL(dewarpStateChanged(int)), ui->playbackBar, SLOT(onFisheyePanelButtonStateChanged(int)));
    connect(BasePlayback::s_fisheyeControl, SIGNAL(controlClosed()), ui->playbackBar, SLOT(onFisheyeDewarpControlClosed()));
#endif

    //
    m_smartSpeedPanel = new SmartSpeedPanel(this);
    m_smartSpeedPanel->hide();
    connect(ui->playbackBar, SIGNAL(smartSpeedPanelButtonClicked(int, int)), m_smartSpeedPanel, SLOT(onSmartSpeedPanelButtonClicked(int, int)));
    connect(m_smartSpeedPanel, SIGNAL(smartSpeedStateChanged(int)), ui->playbackBar, SLOT(onSmartSpeedPanelButtonStateChanged(int)));
    m_filterEventPanel = new FilterEventPanel(this);
    m_filterEventPanel->hide();
    connect(ui->playbackBar, SIGNAL(filterEventPanelButtonClicked(int, int)), m_filterEventPanel, SLOT(onFilterEventPanelButtonClicked(int, int)));
    connect(m_filterEventPanel, SIGNAL(onFilterEventChange()), ui->playbackBar, SLOT(onToolButtionFilterEventSearch()));
    connect(m_filterEventPanel, SIGNAL(onFilterEventChange()), this, SLOT(onToolButtionFilterEventSearch()));
}

PlaybackWindow::~PlaybackWindow()
{
    qDebug() << "Playback::~Playback";
    if (PlaybackRealTimeThread::instance()) {
        PlaybackRealTimeThread::instance()->stopThread();
        delete PlaybackRealTimeThread::instance();
    }
    s_playback = nullptr;
    delete ui;
}

PlaybackWindow *PlaybackWindow::instance()
{
    return s_playback;
}

void PlaybackWindow::initializeData()
{
    qDebug() << QString("Playback::initializeData, begin.");
    //
    if (PlaybackRealTimeThread::instance()) {
        PlaybackRealTimeThread::instance()->startThread();
    }
    qDebug() << "----initialize BasePlayback";
    BasePlayback::initializeBasePlayback(this);
    qDebug() << "----initialize PlaybackMode";
    ui->page_mode->initializeData();
    qDebug() << "----initialize PlaybackBar";
    ui->playbackBar->initializeData();
    //
    if (qMsNvr->is3798()) {
        m_playMode = static_cast<DSP_E>(get_param_int(SQLITE_FILE_NAME, PARAM_STREAM_PLAY_MODE, 0));
        if (m_playMode != DSP_REALTIME) {
            int mode = DSP_REALTIME;
            sendMessageOnly(REQUEST_FLAG_SET_DISPLAY_MODE, &mode, sizeof(mode));
        }
    }
    //
    qDebug() << QString("Playback::initializeData, end.");
}

bool PlaybackWindow::canAutoLogout()
{
    bool ok = true;
    if (BasePlayback::isWaitting()) {
        ok = false;
    }
    if (ui->playbackBar->isRealtimeActive()) {
        ok = false;
    }
    return ok;
}

void PlaybackWindow::showPlayback()
{
    qDebug() << QString("Playback::showPlayback, begin.");
    //
    setGeometry(qApp->desktop()->geometry());
    show();
    //
    //
    qDebug() << QString("Playback::showPlayback, end.");
}

void PlaybackWindow::closePlayback()
{
    if (PlaybackFileManagement::instance()) {
        PlaybackFileManagement::instance()->close();
    }
    m_smartSpeedPanel->closeSmartSpeed();
    BasePlayback::clearDrawTag();
    BasePlayback::s_smartSearch->closeSmartSearch();
    m_filterEventPanel->close();
#ifdef MS_FISHEYE_SOFT_DEWARP
    BasePlayback::s_fisheyeControl->closeFisheyeDewarp();
    BasePlayback::s_fisheyeControl->clearFisheyeChannel(FisheyeDewarpControl::ModePlayback);
    BasePlayback::s_fisheyeControl->clearFisheyeChannel(FisheyeDewarpControl::ModePlaybackSplit);
#endif

    //
    m_isAboutToClose = true;
    QMetaObject::invokeMethod(this, "onClosePlayback", Qt::QueuedConnection);
}

void PlaybackWindow::switchScreen()
{
    qDebug() << QString("Playback::switchScreen");

    //close cut and so on.
    ui->playbackBar->switchScreen();

    //close zoom
    if (PlaybackZoom::instance()->isZooming()) {
        PlaybackZoom::instance()->closeZoom();
    }
    if (PlaybackZoom::instance()->isZoomMode()) {
        PlaybackZoom::instance()->setZoomMode(false);
    }

    //
    ui->page_mode->closePlayback();
}

void PlaybackWindow::closeFisheyeDewarp()
{
#ifdef MS_FISHEYE_SOFT_DEWARP
    BasePlayback::s_fisheyeControl->closeFisheyeDewarp();
#endif
}

void PlaybackWindow::closeFisheyePanel()
{
#ifdef MS_FISHEYE_SOFT_DEWARP
    BasePlayback::s_fisheyeControl->closeFisheyePanel();
#endif
}

QRect PlaybackWindow::playbackBarGlobalGeometry()
{
    return m_playbackBarGeometry;
}

void PlaybackWindow::showNoResource(const QMap<int, bool> &map)
{
    ui->playbackLayout->showNoResource(map);
}

NetworkResult PlaybackWindow::dealNetworkCommond(const QString &commond)
{
    NetworkResult result = NetworkReject;

    if (commond.startsWith("R_Click") || commond.startsWith("Esc")) {
        result = NetworkAccept;
        if (PlaybackZoom::instance()->isZooming()) {
            PlaybackZoom::instance()->closeZoom();
            return result;
        }
        if (PlaybackZoom::instance()->isZoomMode()) {
            PlaybackZoom::instance()->setZoomMode(false);
            return result;
        }
        if (!m_isAboutToClose) {
            closePlayback();
        }
        return result;
    }

    return ui->page_mode->dealNetworkCommond(commond);
}

void PlaybackWindow::dealMessage(MessageReceive *message)
{
    if (!isVisible()) {
        return;
    }

    if (message->isAccepted()) {
        return;
    }
    if (PlaybackFileManagement::s_fileManagement) {
        PlaybackFileManagement::s_fileManagement->dealMessage(message);
    }
    if (message->isAccepted()) {
        return;
    }

    switch (message->type()) {
    case RESPONSE_FLAG_GET_INDENTATION_DIAGRAM:
        ON_RESPONSE_FLAG_GET_INDENTATION_DIAGRAM(message);
        break;
    case RESPONSE_FLAG_STOP_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(message);
        break;
    default:
        break;
    }

    ui->page_mode->dealMessage(message);
    ui->playbackLayout->dealMessage(message);
    ui->playbackBar->dealMessage(message);

    m_smartSpeedPanel->dealMessage(message);
}

void PlaybackWindow::showEvent(QShowEvent *event)
{
    NetworkCommond::instance()->setPageMode(MOD_PLAYBACK);

    setFocus();
    BaseWidget::showEvent(event);
}

void PlaybackWindow::resizeEvent(QResizeEvent *event)
{
    /*******************************
     * 回放显示位置是解码层定义的
     * 所以界面要根据解码层的方式计算界面右侧和底部的大小
     *******************************/
    m_screenGeometry = qApp->desktop()->screenGeometry();
    int rightWidth = m_screenGeometry.width() * 288 / 1920.0;
    int bottomHeight = m_screenGeometry.height() * 122 / 1080.0;

    m_leftGeometry = QRect(m_screenGeometry.left(), m_screenGeometry.top(), m_screenGeometry.width() - rightWidth, m_screenGeometry.height());
    ui->stackedWidget_left->setGeometry(m_leftGeometry);
    ui->stackedWidget_right->setGeometry(m_screenGeometry.right() - rightWidth + 1, m_screenGeometry.top(), rightWidth, m_screenGeometry.height());

    m_layoutGeometry = QRect(m_screenGeometry.left(), m_screenGeometry.top(), m_screenGeometry.width() - rightWidth, m_screenGeometry.height() - bottomHeight);
    ui->playbackLayout->setGeometry(m_layoutGeometry);
    m_layoutSplit->setGeometry(m_layoutGeometry);
    m_playbackBarGeometry = QRect(m_screenGeometry.left(), m_screenGeometry.bottom() - bottomHeight + 1, m_screenGeometry.width() - rightWidth, bottomHeight);
    ui->playbackBar->setGeometry(m_playbackBarGeometry);

    PlaybackZoom::instance()->setGeometry(m_leftGeometry);
    SmartSearchControl::instance()->setGeometry(m_layoutGeometry);

#ifdef MS_FISHEYE_SOFT_DEWARP
    BasePlayback::s_fisheyeControl->setGeometry(m_layoutGeometry);
#endif

    QWidget::resizeEvent(event);
}

void PlaybackWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        m_pressedPoint = event->pos();
        m_isPressed = true;
    }
}

void PlaybackWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPressed && event->button() == Qt::RightButton) {
        if (event->pos() != m_pressedPoint && !PlaybackZoom::instance()->isZooming()) {
            return;
        }
        m_isPressed = false;

        if (BasePlayback::isWaitting()) {
            qMsDebug() << "BasePlayback is waitting.";
            return;
        }
        qDebug() << "Playback::mousePressEvent, Playback right button clicked.";
        if (PlaybackZoom::instance()->isZooming()) {
            PlaybackZoom::instance()->closeZoom();
            return;
        }
        if (PlaybackZoom::instance()->isZoomMode()) {
            PlaybackZoom::instance()->setZoomMode(false);
            return;
        }
        if (SmartSearchControl::instance()->isSmartSearchMode()) {
            SmartSearchControl::instance()->closeSmartSearch();
            return;
        }
        if (!m_isAboutToClose) {
            qDebug() << "Playback::mousePressEvent, Playback is about to close.";
            closePlayback();
        }
    }
}

void PlaybackWindow::wheelEvent(QWheelEvent *event)
{
    if (m_layoutGeometry.contains(event->pos())) {
        if (BasePlayback::playbackState() == PlaybackState_Pause) {
            int numDegrees = event->delta();
            if (numDegrees < 0) {
                //A negative value indicates that the wheel was rotated backwards toward the user.
                if (BasePlayback::playbackType() == SplitPlayback) {
                    PlaybackSplit::instance()->forwardSplitPlayback();
                    PlaybackSplit::instance()->stepSplitPlayback();
                } else {
                    BasePlayback::stepBackwardAllPlayback();
                }
            } else if (numDegrees > 0) {
                //A positive value indicates that the wheel was rotated forwards away from the user.
                if (BasePlayback::playbackType() == SplitPlayback) {
                    PlaybackSplit::instance()->backwardSplitPlayback();
                    PlaybackSplit::instance()->stepSplitPlayback();
                } else {
                    BasePlayback::stepForwardAllPlayback();
                }
            }
        }
    }
}

void PlaybackWindow::escapePressed()
{
    if (!m_isAboutToClose) {
        qDebug() << "Playback::escapePressed, Playback is about to close.";
        closePlayback();
    }
}

void PlaybackWindow::ON_RESPONSE_FLAG_GET_INDENTATION_DIAGRAM(MessageReceive *message)
{
    if (!BasePlayback::s_thumbWidget->isEnabled()) {
        return;
    }

    qDebug() << "RESPONSE_FLAG_GET_INDENTATION_DIAGRAM, data:" << message->data << ", size:" << message->header.size;
    if (message->header.size == -1 || message->data) {
        QImage image;

        if (message->data) {
            image = message->image1;
        } else {
            image = QImage(":/retrieve/retrieve/error_image.png");
        }

        BasePlayback::s_thumbWidget->setImage(image);

        static int w = 300;
        static int h = 200;
        QPoint p = QCursor::pos();
        QRect rc(p.x() - w / 2, m_playbackBarGeometry.top() - h - 1, w, h);
        if (rc.left() < m_leftGeometry.left() + 1) {
            rc.moveLeft(m_leftGeometry.left() + 1);
        }
        if (rc.right() > m_leftGeometry.right() - 1) {
            rc.moveRight(m_leftGeometry.right() - 1);
        }
        BasePlayback::s_thumbWidget->setGeometry(rc);
        if (!BasePlayback::s_thumbWidget->isVisible()) {
            BasePlayback::s_thumbWidget->raise();
            BasePlayback::s_thumbWidget->show();
        }
    } else {
        BasePlayback::s_thumbWidget->hide();
    }
}

void PlaybackWindow::ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)
}

bool PlaybackWindow::isAddToVisibleList()
{
    return true;
}

void PlaybackWindow::onPlaybackModeChanged(MsPlaybackType mode)
{
    switch (mode) {
    case PicturePlayback:
        ui->stackedWidget_left->setCurrentIndex(1);
        ui->page_picture->clear();
        break;
    default:
        ui->stackedWidget_left->setCurrentIndex(0);
        break;
    }
    //
    if (mode == SplitPlayback) {
        m_layoutSplit->show();
        ui->playbackLayout->hide();
    } else {
        m_layoutSplit->hide();
        ui->playbackLayout->show();
    }
}

void PlaybackWindow::onClosePlayback()
{
    qMsDebug() << QString("begin");

    //close cut
    qMsDebug() << QString("close playback cut");
    ui->playbackBar->closePlaybackCut();

    //close zoom
    qMsDebug() << QString("close playback zoom");
    if (PlaybackZoom::instance()->isZooming()) {
        PlaybackZoom::instance()->closeZoom();
    }
    if (PlaybackZoom::instance()->isZoomMode()) {
        PlaybackZoom::instance()->setZoomMode(false);
    }

    //
    qMsDebug() << QString("close playback");
    ui->page_mode->closePlayback();

    //
    if (PlaybackFileManagement::instance()->hasNotExportedVideoFile()) {
        qMsDebug() << QString("check video");
        const int result = MessageBox::question(this, GET_TEXT("PLAYBACK/80093", "Clipping videos have not been exported yet. Still exit?"));
        if (result == MessageBox::Cancel) {
            m_isAboutToClose = false;
            PlaybackFileManagement::instance()->setCurrentMode(PlaybackFileManagement::ModeVideo);
            PlaybackFileManagement::instance()->show();
            qDebug() << QString("Playback::onClosePlayback, cancel.");
            return;
        }
    }
    if (PlaybackFileManagement::instance()->hasNotExportedPictureFile()) {
        qMsDebug() << QString("check picture");
        const int result = MessageBox::question(this, GET_TEXT("PLAYBACK/80094", "Snapshots have not been exported yet. Still exit?"));
        if (result == MessageBox::Cancel) {
            m_isAboutToClose = false;
            PlaybackFileManagement::instance()->setCurrentMode(PlaybackFileManagement::ModeSnapshot);
            PlaybackFileManagement::instance()->show();
            qDebug() << QString("Playback::onClosePlayback, cancel.");
            return;
        }
    }

    //
    qMsDebug() << QString("close audio");
    BasePlayback::closePlaybackAudio();
    PlaybackFileManagement::instance()->cleanup();

    //
    if (ui->playbackBar->isBestDecodeMode()) {
        LiveView::instance()->initializeSubScreenLayout();
    }

    if (PlaybackRealTimeThread::instance()) {
        PlaybackRealTimeThread::instance()->stopThread();
    }

    //
    gPlaybackEventData.clearAll();

    //
    m_isAboutToClose = false;
    BasePlayback::clearupBasePlayback();
    qMsNvr->clearNoResource(SubControl::instance()->currentScreen());
    hide();
    //
    gPlaybackData.resetPlaybackSpeed();
    //
    if (qMsNvr->is3798()) {
        sendMessageOnly(REQUEST_FLAG_SET_DISPLAY_MODE, &m_playMode, sizeof(m_playMode));
    }
    //
    qDebug() << QString("Leave Playback");
    emit playbackClosed();
}

void PlaybackWindow::onToolButtionFilterEventSearch()
{
    m_videoBar->setSmartButtonEnabled(BasePlayback::getFilterEvent() == INFO_MAJOR_NONE);
}
