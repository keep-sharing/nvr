#include "LiveViewTarget.h"
#include "ui_LiveViewTarget.h"
#include "BottomBar.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "TargetPreviewSettings.h"
#include "anprthread.h"
#include "msuser.h"
#include <QElapsedTimer>
#include <QPainter>
#include <QtDebug>
#include "TargetInfoManager.h"

LiveViewTarget *LiveViewTarget::s_liveViewTarget = nullptr;

LiveViewTarget::LiveViewTarget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LiveViewTarget)
{
    ui->setupUi(this);

    s_liveViewTarget = this;
    installEventFilter(this);

    //
    connect(&gTargetInfoManager, SIGNAL(infoChanged()), this, SLOT(onTargetInfoChanged()));
    connect(&gTargetInfoManager, SIGNAL(infoCleared()), this, SLOT(onTargetInfoCleared()));

    //
    m_anprPlay = new LiveViewTargetPlay(this);

    //
    for (int i = 0; i < MAX_TARGET_ITEM_COUNT; ++i) {
        LiveViewTargetItem *item = findChild<LiveViewTargetItem *>(QString("widget_item%1").arg(i));
        if (!item) {
            qWarning() << QString("Can't find LiveViewTargetItem named widget_item%1").arg(i);
            continue;
        }
        item->setIndex(i);
        connect(item, SIGNAL(itemClicked(int)), this, SLOT(onItemClicked(int)));
        connect(item, SIGNAL(itemContextMenuRequested(int, QPoint)), this, SLOT(onItemContextMenuRequested(int, QPoint)));
        m_itemList.append(item);
    }

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

LiveViewTarget::~LiveViewTarget()
{
    s_liveViewTarget = nullptr;
    delete ui;
}

LiveViewTarget *LiveViewTarget::instance()
{
    return s_liveViewTarget;
}

void LiveViewTarget::setTargetEnable(bool enable, int reason)
{
    qDebug() << QString("LiveViewTarget::setAnprEnable, enable: %1").arg(enable);
    m_isEnable = enable;

    gTargetInfoManager.setReceiveEnable(enable);

    if (reason != LiveView::LogoutShowReason) {
        qMsNvr->setTargetMode(m_isEnable);
    }
}

bool LiveViewTarget::isTargetEnable() const
{
    return m_isEnable;
}

void LiveViewTarget::updateTargetrInfo()
{
    //setUpdatesEnabled(false);
    for (int i = 0; i < m_itemList.size(); ++i) {
        LiveViewTargetItem *item = m_itemList.at(i);
        item->updateItemInfo();
    }
    //setUpdatesEnabled(true);
}

void LiveViewTarget::showNoResource(const QMap<int, bool> &map)
{
    if (m_anprPlay) {
        m_anprPlay->showNoResource(map);
    }
}

bool LiveViewTarget::isPlaying()
{
    return m_isPlaying;
}

void LiveViewTarget::setNetworkFocus(bool focus)
{
    m_isNetwokFocus = focus;
    if (focus) {
        networkFocusNext();
    } else {
        m_networkFocusIndex = -1;
        for (int i = 0; i < m_itemList.size(); ++i) {
            LiveViewTargetItem *item = m_itemList.at(i);
            item->setNetworkFocus(false);
        }
    }
}

bool LiveViewTarget::isNetworkFocus()
{
    return m_isNetwokFocus;
}

void LiveViewTarget::networkFocusNext()
{
    if (m_isPlaying) {
        return;
    }

    if (m_networkFocusIndex < 0) {
        m_networkFocusIndex = 0;
    } else if (m_networkFocusIndex < m_itemList.size() - 1) {
        m_networkFocusIndex++;
    }
    for (int i = 0; i < m_itemList.size(); ++i) {
        LiveViewTargetItem *item = m_itemList.at(i);
        if (i == m_networkFocusIndex) {
            item->setNetworkFocus(true);
        } else {
            item->setNetworkFocus(false);
        }
    }
}

void LiveViewTarget::networkFocusPrevious()
{
    if (m_isPlaying) {
        return;
    }

    if (m_networkFocusIndex > 0) {
        m_networkFocusIndex--;
    }
    for (int i = 0; i < m_itemList.size(); ++i) {
        LiveViewTargetItem *item = m_itemList.at(i);
        if (i == m_networkFocusIndex) {
            item->setNetworkFocus(true);
        } else {
            item->setNetworkFocus(false);
        }
    }
}

void LiveViewTarget::hideNetworkFocus()
{
    for (int i = 0; i < m_itemList.size(); ++i) {
        LiveViewTargetItem *item = m_itemList.at(i);
        item->setNetworkFocus(false);
    }
}

void LiveViewTarget::showNetworkFocus()
{
    for (int i = 0; i < m_itemList.size(); ++i) {
        LiveViewTargetItem *item = m_itemList.at(i);
        if (i == m_networkFocusIndex) {
            item->setNetworkFocus(true);
        } else {
            item->setNetworkFocus(false);
        }
    }
}

void LiveViewTarget::networkPlay()
{
    if (m_anprPlay->isVisible()) {

    } else {
        QMetaObject::invokeMethod(this, "onItemClicked", Qt::QueuedConnection, Q_ARG(int, m_networkFocusIndex));
    }
}

bool LiveViewTarget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void LiveViewTarget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    gTargetInfoManager.updatePlateType();
}

void LiveViewTarget::paintEvent(QPaintEvent *)
{
#ifdef _NT98323_
    QPainter painter(this);
    QRegion region = QRegion(rect()) - QRegion(m_videoRect);
    QPainterPath painterPath;
    painterPath.addRegion(region);
    painter.fillPath(painterPath, QColor(22, 22, 22));

#endif
}

void LiveViewTarget::addAnprlist(const QString &plate, const QString &type)
{
    qDebug() << QString("LiveViewTarget::addAnprlist, plate: %1, type: %2").arg(plate).arg(type);

    int anprListCount = read_anpr_list_cnt(SQLITE_ANPR_NAME);

    anpr_list anpr_info;
    memset(&anpr_info, 0, sizeof(anpr_list));
    read_anpr_list_plate(SQLITE_ANPR_NAME, &anpr_info, plate.toStdString().c_str());
    if (!QString(anpr_info.plate).isEmpty()) {
        int result = MessageBox::question(this, GET_TEXT("ANPR/103045", "Detected the same plate(s). Do you want to replace the old ones?"));
        if (result == MessageBox::Cancel) {
            return;
        } else {
            snprintf(anpr_info.plate, sizeof(anpr_info.plate), "%s", plate.toStdString().c_str());
            snprintf(anpr_info.type, sizeof(anpr_info.type), "%s", type.toStdString().c_str());
            update_anpr_list(SQLITE_ANPR_NAME, &anpr_info);
        }
    } else {
        if (anprListCount >= MAX_ANPR_LIST_COUNT) {
            ShowMessageBox(QString(GET_TEXT("ANPR/103046", "Out of capacity！Successful:%1 Failed:%2")).arg(0).arg(1));
            return;
        }
        snprintf(anpr_info.plate, sizeof(anpr_info.plate), "%s", plate.toStdString().c_str());
        snprintf(anpr_info.type, sizeof(anpr_info.type), "%s", type.toStdString().c_str());
        write_anpr_list(SQLITE_ANPR_NAME, &anpr_info);
    }
}

void LiveViewTarget::removeAnprList(const QString &plate)
{
    qDebug() << QString("LiveViewTarget::removeAnprList, plate: %1").arg(plate);

    anpr_list anpr_info;
    memset(&anpr_info, 0, sizeof(anpr_list));
    read_anpr_list_plate(SQLITE_ANPR_NAME, &anpr_info, plate.toStdString().c_str());
    if (QString(anpr_info.plate).isEmpty()) {
        ShowMessageBox(GET_TEXT("ANPR/103071", "Not existed in Black or White list！"));
        return;
    }

    const int result = MessageBox::question(this, GET_TEXT("ANPR/103070", "Do you really want to delete present license(s) from B/W list?"));
    if (result == MessageBox::Cancel) {
        return;
    }

    snprintf(anpr_info.plate, sizeof(anpr_info.plate), "%s", plate.toStdString().c_str());
    delete_anpr_list(SQLITE_ANPR_NAME, &anpr_info);
}

void LiveViewTarget::updateItemType()
{
    gTargetInfoManager.updatePlateType();
}

void LiveViewTarget::clearTargetInfo()
{
    gTargetInfoManager.clearTargetInfo();
}

void LiveViewTarget::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("TARGETMODE/103207", "Target Preview"));
}

void LiveViewTarget::onItemClicked(int index)
{
    qDebug() << QString("LiveViewTarget::onItemClicked, index: %1").arg(index);

    //
    if (index < 0) {
        qCritical() << QString("LiveViewTarget::onItemClicked, index: %1").arg(index);
        return;
    }

    //
    LiveViewTargetItem *clickedItem = m_itemList.at(index);
    int channel = clickedItem->channel();
    if (channel < 0) {
        return;
    }

    bool bottomBarVisible = false;
    do {
        QRect rc = clickedItem->globalGeometry();

        m_videoRect = rc;
        update();

        m_isPlaying = true;
        hideNetworkFocus();

        //临时隐藏BottomBar
        if (index > 2) {
            bottomBarVisible = BottomBar::instance()->isVisible();
            if (bottomBarVisible) {
                BottomBar::instance()->hide();
            }
        }

        //
        clickedItem->setTempHide(true, false);
        //查询录像
        QDateTime dateTime = clickedItem->anprTime();
        int searchResult = m_anprPlay->waitForSearchAnprPlayback(channel, dateTime, rc);
        if (searchResult < 0) {
            ShowMessageBox(GET_TEXT("DISK/92033", "No record files currently."));
            break;
        }
        m_anprPlay->execAnprPlayback(rc);
    } while (0);

    //
    clickedItem->setTempHide(false);

    //
    updateTargetrInfo();

    //恢复BottomBar
    if (bottomBarVisible) {
        BottomBar::instance()->animateShow();
    }

    m_isPlaying = false;
    showNetworkFocus();

    m_videoRect = QRect();
    update();
}

void LiveViewTarget::onItemContextMenuRequested(int index, const QPoint &pos)
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_TARGETMODEOPERATION )) {
        ShowMessageBox(this, GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    if (qMsNvr->isSlaveMode()) {
        return;
    }
    //
    LiveViewTargetItem *item = m_itemList.at(index);
    if (!item->hasInfo()) {
        return;
    }
    if (item->infoType() != TargetInfo::TARGET_ANPR) {
        return;
    }

    if (!m_itemMenu) {
        m_itemMenu = new AnprItemMenu(this);
        connect(m_itemMenu, SIGNAL(addBlacklist(QString)), this, SLOT(onAddToBlacklist(QString)));
        connect(m_itemMenu, SIGNAL(addWhitelist(QString)), this, SLOT(onAddToWhitelist(QString)));
        connect(m_itemMenu, SIGNAL(removeFromList(QString)), this, SLOT(onRemoveFromList(QString)));
    }
    m_itemMenu->setPlate(item->plate());
    m_itemMenu->exec(pos);
}

void LiveViewTarget::onTargetInfoChanged()
{
    //播放时不推送新的信息显示在live
    if (m_isPlaying) {
        return;
    }
    if (!isVisible()) {
        return;
    }
    updateTargetrInfo();
}

void LiveViewTarget::onTargetInfoCleared()
{
    setUpdatesEnabled(false);
    for (int i = 0; i < m_itemList.size(); ++i) {
        LiveViewTargetItem *item = m_itemList.at(i);
        item->clearItemInfo();
    }
    setUpdatesEnabled(true);
}

void LiveViewTarget::onAddToBlacklist(const QString &plate)
{
    addAnprlist(plate, QString(PARAM_MS_ANPR_TYPE_BLACK));
}

void LiveViewTarget::onAddToWhitelist(const QString &plate)
{
    addAnprlist(plate, QString(PARAM_MS_ANPR_TYPE_WHITE));
}

void LiveViewTarget::onRemoveFromList(const QString &plate)
{
    removeAnprList(plate);
}

void LiveViewTarget::on_toolButtonSettings_clicked()
{
    ui->toolButtonSettings->clearUnderMouse();

    TargetPreviewSettings settings(this);
    settings.exec();
}
