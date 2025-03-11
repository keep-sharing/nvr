#include "FisheyeDewarpControl.h"
#include "ui_FisheyeDewarpControl.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include "PlaybackData.h"
#include "BottomBar.h"
#include "FisheyeDewarpBottomBar.h"
#include "SubControl.h"
#include <QCursor>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>

#define FISHEYE_MAX_OUT_W (1920)
#define FISHEYE_MAX_OUT_H (1080)

QDebug operator<<(QDebug debug, const MSPointi &p)
{
    debug.nospace() << '(' << p.x << ", " << p.y << ')';
    return debug;
}

QList<FisheyeDewarpControl *> gControlList;
int FisheyeDewarpControl::s_fisheyeControlMode = FisheyeDewarpControl::ModeNone;

FisheyeDewarpControl::FisheyeDewarpControl(QWidget *parent)
    : MainDialog(parent)
    , ui(new Ui::FisheyeDewarpControl)
{
    gControlList.append(this);
    ui->setupUi(this);

    setWindowFlags(Qt::Widget);
    setMouseTracking(true);

    m_fishPanel = new FisheyePanel(parent);
    m_fishPanel->hide();
    connect(m_fishPanel, SIGNAL(dewarpStateChanged(int)), this, SLOT(onDewarpStateChanged(int)));

    connect(qMsNvr, SIGNAL(fisheyeHandleChanged()), this, SLOT(onFisheyeHandleChanged()));
}

FisheyeDewarpControl::~FisheyeDewarpControl()
{
    gControlList.removeOne(this);
    delete ui;
}

void FisheyeDewarpControl::setCurrentChannel(FisheyeKey key)
{
    FisheyePanel::s_currentFisheyeKey = key;
}

FisheyeKey FisheyeDewarpControl::currentChannel()
{
    return FisheyePanel::s_currentFisheyeKey;
}

void FisheyeDewarpControl::setVapiWinId(int id)
{
    FisheyePanel::s_vapiWinId = id;
}

FisheyeKey FisheyeDewarpControl::fisheyeChannel()
{
    return FisheyePanel::fisheyeChannel();
}

FisheyeKey FisheyeDewarpControl::fisheyeChannel(FisheyeDewarpControl::Mode mode)
{
    return FisheyePanel::fisheyeChannel(mode);
}

void FisheyeDewarpControl::clearFisheyeChannel()
{
    FisheyePanel::clearFisheyeChannel();
}

void FisheyeDewarpControl::clearFisheyeChannel(FisheyeDewarpControl::Mode mode)
{
    FisheyePanel::clearFisheyeChannel(mode);
}

void FisheyeDewarpControl::setFisheyeChannel(FisheyeKey key)
{
    FisheyePanel::setFisheyeChannel(key);
}

void FisheyeDewarpControl::setFisheyeChannel(FisheyeDewarpControl::Mode mode, FisheyeKey key)
{
    FisheyePanel::setFisheyeChannel(mode, key);
}

int FisheyeDewarpControl::fisheyeDisplayMode(FisheyeKey key)
{
    return FisheyePanel::fisheyeDisplayMode(key);
}

void FisheyeDewarpControl::callbackWireFrameTouch(void *, int value, char *name)
{
    Q_UNUSED(name)

    for (int i = 0; i < gControlList.size(); ++i) {
        FisheyeDewarpControl *control = gControlList.at(i);
        QMetaObject::invokeMethod(control, "setFisheyeCursor", Q_ARG(int, value));
    }
}

int FisheyeDewarpControl::execFisheyeDewarpControl(FisheyeKey key)
{
    s_fisheyeControlMode = key.mode;
    m_mode = key.mode;
    FisheyePanel::s_controlMode = key.mode;
    show();

    if (currentChannel() == fisheyeChannel()) {
        setWireFrameEnable(true);
    }
    //
    //int result = m_eventLoop.exec();
    //
    if (currentChannel() == fisheyeChannel()) {
        setWireFrameEnable(false);
    }
    return 0;
}

void FisheyeDewarpControl::showFisheyeDewarpControl(FisheyeKey key)
{
    s_fisheyeControlMode = key.mode;
    m_mode = key.mode;
    FisheyePanel::s_controlMode = key.mode;
    show();

    if (currentChannel() == fisheyeChannel()) {
        setWireFrameEnable(true);
    }

    //
    if (key.mode == ModePlayback || key.mode == ModePlaybackSplit) {
        gPlaybackData.setFisheyeMode(true);
    }
}

void FisheyeDewarpControl::hideFisheyeDewarpControl()
{
    m_fishPanel->hide();
    hide();
}

void FisheyeDewarpControl::showEvent(QShowEvent *)
{
    //
    int nvrWidth = 0;
    int nvrHeight = 0;
    vapi_get_screen_res(SubControl::instance()->currentScreen(), &nvrWidth, &nvrHeight);
    m_nvrScreenSize = QSize(qMin(nvrWidth, FISHEYE_MAX_OUT_W), qMin(nvrHeight, FISHEYE_MAX_OUT_H));
    //
    QRect screenRect = qApp->desktop()->geometry();
    m_qtScreenSize = screenRect.size();
}

void FisheyeDewarpControl::paintEvent(QPaintEvent *)
{
    //    QPainter painter(this);
    //    painter.setPen(Qt::NoPen);
    //    painter.setBrush(QColor(255, 0, 0, 80));
    //    painter.drawRect(rect());
}

void FisheyeDewarpControl::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressedPoint = event->pos();

        if (m_fishPanel->isDewarpEnabled()) {
            qMsNvr->fisheyeHandleLock();
            if (qMsNvr->s_fisheyeHandle) {
                MSPointi p = makeMsPoint(event->pos());
                qDebug() << "FisheyePlayer_OnLButtonDown, flag: 1"
                         << ", pos:" << p;
                FisheyePlayer_OnLButtonDown(qMsNvr->s_fisheyeHandle, 1, p);
            }
            qMsNvr->fisheyeHandleUnlock();
        }
    } else if (event->button() == Qt::RightButton) {
        qDebug() << "FisheyeDewarpControl::mousePressEvent, Qt::RightButton";
        closeFisheyeDewarp();
    }
}

void FisheyeDewarpControl::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;

    if (m_fishPanel->isDewarpEnabled()) {
        qMsNvr->fisheyeHandleLock();
        if (qMsNvr->s_fisheyeHandle) {
            MSPointi p = makeMsPoint(event->pos());
            qDebug() << "FisheyePlayer_OnLButtonUp, flag: 1"
                     << ", pos:" << p;
            FisheyePlayer_OnLButtonUp(qMsNvr->s_fisheyeHandle, 1, p);
        }
        qMsNvr->fisheyeHandleUnlock();
    }
}

void FisheyeDewarpControl::mouseMoveEvent(QMouseEvent *event)
{
    if (m_fishPanel->isDewarpEnabled()) {
        qMsNvr->fisheyeHandleLock();
        if (qMsNvr->s_fisheyeHandle) {
            MSPointi p = makeMsPoint(event->pos());
            //qDebug() << "FisheyePlayer_OnMouseMove, flag:" << (m_pressed ? 1 : 0) << ", pos:" << event->pos();
            FisheyePlayer_OnMouseMove(qMsNvr->s_fisheyeHandle, m_pressed ? 1 : 0, p);
        }
        qMsNvr->fisheyeHandleUnlock();
    }
}

void FisheyeDewarpControl::wheelEvent(QWheelEvent *event)
{
    if (m_fishPanel->isDewarpEnabled()) {
        qMsNvr->fisheyeHandleLock();
        if (qMsNvr->s_fisheyeHandle) {
            int numDegrees = event->delta() / 8;
            MSPointi p = makeMsPoint(event->pos());
            qDebug() << "FisheyePlayer_OnMouseWheel, flag: 0"
                     << ", delta:" << -numDegrees << ", pos:" << p;
            FisheyePlayer_OnMouseWheel(qMsNvr->s_fisheyeHandle, 0, -numDegrees, p);
        }
        qMsNvr->fisheyeHandleUnlock();
    }
}

void FisheyeDewarpControl::setWireFrameEnable(bool enable)
{
    if (m_fishPanel->isDewarpEnabled()) {
        qMsNvr->fisheyeHandleLock();
        if (qMsNvr->s_fisheyeHandle) {
            qMsDebug() << "\n----FisheyePlayer_EnableWireFrame, show frame: true";
            FisheyePlayer_EnableWireFrame(qMsNvr->s_fisheyeHandle, enable);
        }
        qMsNvr->fisheyeHandleUnlock();
    }
}

MSPointi FisheyeDewarpControl::makeMsPoint(const QPoint &p)
{
    MSPointi msPoint;
    if (m_qtScreenSize == m_nvrScreenSize) {
        msPoint.x = p.x();
        msPoint.y = p.y();
    } else {
        msPoint.x = (qreal)p.x() * m_nvrScreenSize.width() / m_qtScreenSize.width();
        msPoint.y = (qreal)p.y() * m_nvrScreenSize.height() / m_qtScreenSize.height();
    }
    return msPoint;
}

void FisheyeDewarpControl::onLiveViewFisheyePanelButtonClicked(int x, int y)
{
    if (!m_fishPanel->isVisible()) {
        m_fishPanel->initializePanel();
        m_fishPanel->show();
        QPoint p;
        p.setX(x - m_fishPanel->width() / 2);
        p.setY(y - m_fishPanel->height());
        m_fishPanel->move(p);
    } else {
        m_fishPanel->hide();
    }
}

void FisheyeDewarpControl::onPlaybackFisheyePanelButtonClicked(int x, int y)
{
    if (!m_fishPanel->isVisible()) {
        m_fishPanel->initializePanel();
        m_fishPanel->show();
        QPoint p;
        p.setX(x);
        p.setY(y - m_fishPanel->height());
        m_fishPanel->move(p);
    } else {
        m_fishPanel->hide();
    }
}

void FisheyeDewarpControl::closeFisheyePanel()
{
    m_fishPanel->hide();
}

void FisheyeDewarpControl::closeFisheyeDewarp()
{
    setWireFrameEnable(false);
    //m_fishPanel->closeDewarp();
    m_fishPanel->hide();
    close();

    // if (m_eventLoop.isRunning()) {
    //     m_eventLoop.exit();
    // }

    emit controlClosed();
}

void FisheyeDewarpControl::setFisheyeCursor(int type)
{
    if (type == m_cursorType) {
        return;
    }

    m_cursorType = type;
    switch (m_cursorType) {
    case 0: //不可拖动
        unsetCursor();
        break;
    case 1: //可拖动
        setCursor(Qt::OpenHandCursor);
        break;
    case 2: //按下
        setCursor(Qt::ClosedHandCursor);
        break;
    default:
        unsetCursor();
        break;
    }
}

void FisheyeDewarpControl::onFisheyeHandleChanged()
{
    if (s_fisheyeControlMode == m_mode) {
        qMsNvr->fisheyeHandleLock();
        //设置默认安装模式
        if (qMsNvr->s_fisheyeHandle) {
            const PanelState &state = FisheyePanel::panelState();
            qMsDebug() << "\n----FisheyePlayer_SetInstallModel:" << FisheyeInstallModeString_SoftDewarp(state.installation);
            FisheyePlayer_SetInstallModel(qMsNvr->s_fisheyeHandle, static_cast<Fisheye_InstallModel>(state.installation));
            if (isVisible()) {
                qMsDebug() << "\n----FisheyePlayer_EnableWireFrame, show frame: true";
                FisheyePlayer_EnableWireFrame(qMsNvr->s_fisheyeHandle, true);
            }
            //
            FisheyePlayer_WireFrameTouchedFunc(qMsNvr->s_fisheyeHandle, callbackWireFrameTouch, nullptr);
        }
        qMsNvr->fisheyeHandleUnlock();
    }
}

void FisheyeDewarpControl::onDewarpStateChanged(int state)
{
    emit dewarpStateChanged(state);
}
