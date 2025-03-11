#include "BottomBar.h"
#include "ui_BottomBar.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "Disturbing.h"
#include "LiveView.h"
#include "LiveViewTarget.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "msuser.h"
#include "PopupContent.h"
#include "ptz3dcontrol.h"
#include "SubControl.h"
#include "userlogin.h"
#include <QDateTime>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QTimer>
#include <QtDebug>

BottomBar *BottomBar::s_bottomBar = nullptr;

BottomBar::BottomBar(QWidget *parent)
    : BaseWidget(parent)
    , ui(new Ui::BottomBar)
{
    ui->setupUi(this);
    s_bottomBar = this;
    installEventFilter(this);

    m_animation = new QPropertyAnimation(this, "pos");
    connect(m_animation, SIGNAL(finished()), this, SLOT(animationFinished()));
    m_animation->setDuration(0);

    QDesktopWidget *desktop = qApp->desktop();
    QRect screenRect = desktop->screenGeometry();
    m_leaveRect = QRect(screenRect.left(), screenRect.height() / 4 * 3, screenRect.width(), screenRect.height() / 4);
    m_enterRect = QRect(screenRect.left(), screenRect.height() - 20, screenRect.width(), 22);

    m_timerTime = new QTimer(this);
    connect(m_timerTime, SIGNAL(timeout()), this, SLOT(updateCurrentTime()));
    m_timerTime->start(1000);
    updateCurrentTime();

    //
    m_locked = get_param_int(SQLITE_FILE_NAME, PARAM_GUI_BOTTOMBAR_STATE, 0);

    //
    m_liveviewBar = new LiveviewBottomBar(this);
    m_liveviewBar->hide();
    m_ptzBar = new PtzBottomBar(this);
    m_ptzBar->hide();
#ifdef MS_FISHEYE_SOFT_DEWARP
    m_fisheyeDewarpBar = new FisheyeDewarpBottomBar(this);
    m_fisheyeDewarpBar->hide();
#else
    m_fisheyeBar = new FisheyeBottomBar(this);
    m_fisheyeBar->hide();
#endif

    //
    resize(screenRect.width(), m_height);
    setBottomMode(ModeNormal);

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

BottomBar::~BottomBar()
{
    qDebug() << "BottomBar::~BottomBar()";
    delete ui;
}

BottomBar *BottomBar::instance()
{
    return s_bottomBar;
}

void BottomBar::setBottomMode(BottomBar::BottomMode mode, int channel)
{
    QLayoutItem *item = ui->gridLayout_bar->itemAtPosition(0, 0);
    if (item) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->hide();
        }
        ui->gridLayout_bar->removeItem(item);
    }
    //
    QWidget *widget = nullptr;
    m_mode = mode;
    switch (mode) {
    case ModeNormal:
        widget = m_liveviewBar;
        break;
    case ModePTZ3D:
        widget = m_ptzBar;
        m_ptzBar->initializeData(channel);
        setLocked(true);
        break;
    case ModeFisheye:
#ifdef MS_FISHEYE_SOFT_DEWARP
        widget = m_fisheyeDewarpBar;
        m_fisheyeDewarpBar->initializeData(channel);
#else
        widget = m_fisheyeBar;
#endif
        setLocked(true);
        break;
    }
    if (widget) {
        ui->gridLayout_bar->addWidget(widget, 0, 0);
        widget->show();
    }
}

void BottomBar::initializeState()
{
}

void BottomBar::setLocked(bool lock)
{
    m_locked = lock;
    if (m_locked) {
        if (!isVisible()) {
            animateShow();
        } else {
            raise();
        }
    }
    //
    ui->toolButton_lock->setChecked(lock);
    if (m_locked) {
        ui->toolButton_lock->setToolTip(GET_TEXT("LIVEVIEW/20059", "UnLock"));
    } else {
        ui->toolButton_lock->setToolTip(GET_TEXT("LIVEVIEW/20060", "Lock"));
    }
    //
    set_param_int(SQLITE_FILE_NAME, PARAM_GUI_BOTTOMBAR_STATE, m_locked);
}

bool BottomBar::isLocked()
{
    return m_locked;
}

void BottomBar::showOrHide(const QPoint &point)
{
    if (m_locked && isVisible()) {
        return;
    }

    if (isVisible()) {
        if (mouseLeave(point)) {
            animateHide();
        }
    } else {
        //ANPR模式下左侧不呼出底部菜单栏
        if (LiveViewTarget::instance() && LiveViewTarget::instance()->isTargetEnable()) {
            if (point.x() < width() / 4) {
                return;
            }
        }
        //
        if (mouseEnter(point)) {
            animateShow();
        }
    }
}

bool BottomBar::mouseEnter(const QPoint &mousePos)
{
    return m_enterRect.contains(mousePos);
}

bool BottomBar::mouseLeave(const QPoint &mousePos)
{
    return !m_leaveRect.contains(mousePos);
}

void BottomBar::onLanguageChanged()
{
    if (ui->toolButton_lock->isChecked()) {
        ui->toolButton_lock->setToolTip(GET_TEXT("LIVEVIEW/20059", "UnLock"));
    } else {
        ui->toolButton_lock->setToolTip(GET_TEXT("LIVEVIEW/20060", "Lock"));
    }
}

void BottomBar::animateShow()
{
    if (isVisible()) {
        return;
    }

    QDesktopWidget *desktop = qApp->desktop();
    QRect screenRect = desktop->screenGeometry();
    m_animation->setStartValue(QPoint(screenRect.left(), screenRect.bottom() + 1));
    m_animation->setEndValue(QPoint(screenRect.left(), screenRect.bottom() - m_height + 1));
    m_animation->start();
    m_readyHide = false;
    show();
    raise();
}

void BottomBar::animateHide()
{
    if (!isVisible()) {
        return;
    }

    m_readyHide = true;
    QDesktopWidget *desktop = qApp->desktop();
    QRect screenRect = desktop->screenGeometry();
    m_animation->setStartValue(QPoint(screenRect.left(), screenRect.bottom() - m_height + 1));
    m_animation->setEndValue(QPoint(screenRect.left(), screenRect.bottom() + 1));
    m_animation->start();
}

void BottomBar::tempHide()
{
    if (!isLocked()) {
        hide();
        return;
    }
    m_isTempHide = true;
    animateHide();
}

void BottomBar::resume()
{
    if (m_isTempHide) {
        setLocked(true);
        m_isTempHide = false;
    }
}

MyToolButton *BottomBar::lockButton()
{
    return ui->toolButton_lock;
}

LiveviewBottomBar *BottomBar::liveviewBottomBar()
{
    return m_liveviewBar;
}

void BottomBar::dealMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_EXCEPTION_STATUS:
    case RESPONSE_FLAG_INFOMATION_SHOW:
        break;
    default:
        if (!LiveView::instance()->isVisible()) {
            return;
        }
        break;
    }

    m_liveviewBar->dealMessage(message);
}

int BottomBar::mode() const
{
    return m_mode;
}

bool BottomBar::eventFilter(QObject *obj, QEvent *evt)
{
    switch (evt->type()) {
    case QEvent::ContextMenu:
    case QEvent::Wheel:
        return true;
        break;
    default:
        break;
    }
    return BaseWidget::eventFilter(obj, evt);
}

void BottomBar::showEvent(QShowEvent *event)
{
    emit visibleChanged(true);
    BaseWidget::showEvent(event);
}

void BottomBar::hideEvent(QHideEvent *event)
{
    emit visibleChanged(false);
    BaseWidget::hideEvent(event);
}

bool BottomBar::isAddToVisibleList()
{
    return true;
}

void BottomBar::returnPressed()
{
    QToolButton *button = qobject_cast<QToolButton *>(focusWidget());
    if (button) {
        qDebug() << "BottomBar::returnPressed," << button;
        button->click();
    }
}

void BottomBar::animationFinished()
{
    if (m_readyHide) {
        hide();
    }
}

void BottomBar::updateCurrentTime()
{
    ui->label_time->setText(QTime::currentTime().toString("HH:mm:ss"));
    ui->label_date->setText(QDate::currentDate().toString("yyyy-MM-dd"));
}

void BottomBar::on_toolButton_lock_clicked(bool checked)
{
    setLocked(checked);
}
