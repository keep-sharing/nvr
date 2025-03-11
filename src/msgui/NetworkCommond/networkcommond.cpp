#include "networkcommond.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "LiveView.h"
#include "PlaybackWindow.h"
#include "Sequence.h"
#include "settingcontent.h"
#include <QApplication>
#include <QMouseEvent>
#include <QStringList>
#include <linux/input.h>

extern "C" {
#include "ptz_public.h"
}

QDebug operator<<(QDebug debug, const NetworkResult &r)
{
    switch (r) {
    case NetworkReject:
        debug.nospace() << "NetworkReject";
        break;
    case NetworkAccept:
        debug.nospace() << "NetworkAccept";
        break;
    case NetworkTakeOver:
        debug.nospace() << "NetworkTakeOver";
        break;
    default:
        debug.nospace() << "NetworkResult(" << r << ")";
        break;
    }

    return debug.space();
}

NetworkCommond *NetworkCommond::s_networkCommond = nullptr;
NetworkCommond::NetworkCommond(QObject *parent)
    : MsObject(parent)
{
    s_networkCommond = this;

    m_commondThread = new CommondThread();

#ifdef Keyboard_Simulation
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(onTcpNewConnection()));
    bool ok = m_tcpServer->listen(QHostAddress::Any, 4567);
    if (!ok) {
        qWarning() << "NetworkCommond listen," << m_tcpServer->errorString();
    }
#endif
}

NetworkCommond::~NetworkCommond()
{
    qDebug() << "NetworkCommond::~NetworkCommond()";
    m_commondThread->stopThread();
    delete m_commondThread;
    m_commondThread = nullptr;
    s_networkCommond = nullptr;
}

NetworkCommond *NetworkCommond::instance()
{
    return s_networkCommond;
}

void NetworkCommond::setPageMode(PageMode mode)
{
    m_mode = mode;
}

void NetworkCommond::sendKeyEvent(int key, KeyEventType type)
{
    m_commondThread->sendKeyEvent(key, type);
}

void NetworkCommond::ON_RESPONSE_FLAG_SENDTO_QT_CMD(MessageReceive *message)
{
    struct sendto_qt_cmd *commond = (struct sendto_qt_cmd *)message->data;
    if (!commond) {
        qWarning() << "NetworkCommond::ON_RESPONSE_FLAG_SENDTO_QT_CMD, data is null.";
        return;
    }
    const QString strCommond(commond->cmd);
    qDebug() << QString("NetworkCommond::ON_RESPONSE_FLAG_SENDTO_QT_CMD, cmd: %1, reqfrom: %2").arg(strCommond).arg(commond->reqfrom);

    //
    bool result = dealCommond(strCommond);

    //
    struct get_qt_cmd_result param;
    memset(&param, 0, sizeof(param));
    param.reqfrom = commond->reqfrom;
    param.clientId = commond->clientId;
    if (result) {
        strcpy(param.result, "OK");
    } else {
        strcpy(param.result, "INVALID");
    }
    strcpy(param.cmd, commond->cmd);
    snprintf(param.curPage, sizeof(param.curPage), "%s", currentModeName().toStdString().c_str());
    qDebug() << QString("NetworkCommond::ON_RESPONSE_FLAG_SENDTO_QT_CMD, REQUEST_FLAG_GET_QT_CMD_RESULT");
    sendMessageOnly(REQUEST_FLAG_GET_QT_CMD_RESULT, (void *)&param, sizeof(param));
}

bool NetworkCommond::dealCommond(const QString &strCommond)
{
    bool result = false;
    if (strCommond.startsWith("KeyDown_")) {
        result = cmd_KeyEvent(strCommond);
    } else if (strCommond.startsWith("KeyUp_")) {
        result = cmd_KeyEvent(strCommond);
    } else if (strCommond.startsWith("Key_")) {
        result = cmd_KeyEvent(strCommond);
    } else if (strCommond.startsWith("Enter")) {
        result = cmd_Enter(strCommond);
    } else if (strCommond.startsWith("Esc")) {
        result = cmd_Esc(strCommond);
    } else if (strCommond.startsWith("TabPage_Next")) {
        //T1
        result = cmd_TabPage(strCommond);
    } else if (strCommond.startsWith("TabPage_Prev")) {
        //反向T1
        result = cmd_TabPage_Prev(strCommond);
    } else if (strCommond.startsWith("ChangeFocus_Next")) {
        //T2
        result = cmd_ChangeFocus(strCommond);
    } else if (strCommond.startsWith("ChangeFocus_Prev")) {
        //反向T2
        result = cmd_ChangeFocus_Prev(strCommond);
    } else if (strCommond.startsWith("SelWnd_")) {
        //网络键盘：WIN
        //SelWnd_3
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Wnd_")) {
        //网络键盘：MULT，切换布局
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Page_")) {
        result = cmd_Page(strCommond);
    } else if (strCommond.startsWith("SelChannel_")) {
        //网络键盘：CAM
        //SelChannel_1
        result = cmd_SelChannel(strCommond);
    } else if (strCommond.startsWith("SingleChannel_")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Screen_")) {
        //网络键盘：PREV，NEXT
        //Screen_Prev
        //Screen_Next
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("FullScreen")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Audio")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Rec")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Snap")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("PlayBack_")) {
        result = cmd_PlayBack(strCommond);
    } else if (strCommond.startsWith("Mouse_")) {
        result = cmd_Mouse(strCommond);
    } else if (strCommond.startsWith("Menu")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Seq_")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("PTZ_")) {
        result = cmd_Ptz(strCommond);
    } else if (strCommond.startsWith("Dir_")) {
        result = cmd_Dir(strCommond);
    } else if (strCommond.startsWith("GetStatusInfo")) {

    } else if (strCommond.startsWith("CloseDialog")) {

    } else if (strCommond.startsWith("Video_Play")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Video_Pause")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Video_Stop")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Video_Forward")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Video_Backward")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Video_Here")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Dial_Out_Add")) {
        if (m_mode == MOD_PLAYBACK) {
            result = cmd_GeneralResolver(strCommond);
        } else {
            //T1
            result = cmd_TabPage(strCommond);
        }
    } else if (strCommond.startsWith("Dial_Out_Sub")) {
        if (m_mode == MOD_PLAYBACK) {
            result = cmd_GeneralResolver(strCommond);
        } else {
            //T1
            result = cmd_TabPage_Prev(strCommond);
        }
    } else if (strCommond.startsWith("Dial_Insid_Add")) {
        if (m_mode == MOD_PLAYBACK) {
            result = cmd_GeneralResolver(strCommond);
        } else {
            //T2
            result = cmd_ChangeFocus(strCommond);
        }
    } else if (strCommond.startsWith("Dial_Insid_Sub")) {
        if (m_mode == MOD_PLAYBACK) {
            result = cmd_GeneralResolver(strCommond);
        } else {
            //T2
            result = cmd_ChangeFocus_Prev(strCommond);
        }
    } else if (strCommond.startsWith("R_Click")) {
        result = cmd_GeneralResolver(strCommond);
    } else if (strCommond.startsWith("Toolbar")) {
        result = cmd_GeneralResolver(strCommond);
    } else {
        qWarning() << "Unhandled commond:" << strCommond;
    }
    return result;
}

bool NetworkCommond::cmd_KeyEvent(const QString &strCommond)
{
    QStringList strList = strCommond.split("_");
    if (strList.size() != 2) {
        return false;
    }
    const QString strType = strList.at(0);
    const QString strKey = strList.at(1);
    const int key = strKey.toInt();
    if (strType == "KeyDown") {
        m_commondThread->sendKeyEvent(key, KeyPress);
    } else if (strType == "KeyUp") {
        m_commondThread->sendKeyEvent(key, KeyRelease);
    } else if (strType == "Key") {
        m_commondThread->sendKeyEvent(key, KeyClick);
    } else {
        return false;
    }
    return true;
}

/**
 * @brief NetworkCommond::cmd_Enter
 * 网络键盘：摇杆顶部按键
 * @param strCommond
 * @return
 */
bool NetworkCommond::cmd_Enter(const QString &strCommond)
{
    NetworkResult result = NetworkReject;

    QList<BaseWidget *> widgetList = BaseWidget::visibleList();
    for (int i = 0; i < widgetList.size(); ++i) {
        BaseWidget *widget = widgetList.at(i);
        result = widget->dealNetworkCommond(strCommond);
        if (result == NetworkAccept || result == NetworkTakeOver) {
            break;
        }
    }

    //
    if (result == NetworkReject || result == NetworkTakeOver) {
        m_commondThread->sendKeyEvent(KEY_ENTER, KeyClick);
        result = NetworkAccept;
    }
    return result == NetworkAccept;
}

bool NetworkCommond::cmd_Page(const QString &strCommond)
{
    Q_UNUSED(strCommond)

    return false;
}

bool NetworkCommond::cmd_SelChannel(const QString &strCommond)
{
    Q_UNUSED(strCommond)

    return false;
}

bool NetworkCommond::cmd_PlayBack(const QString &strCommond)
{
    Q_UNUSED(strCommond)

    return false;
}

bool NetworkCommond::cmd_Mouse(const QString &strCommond)
{
    Q_UNUSED(strCommond)

    //Mouse_move_x_x
    QRegExp rx("Mouse_(.+)_(.+)_(.+)");
    if (rx.indexIn(strCommond) == -1) {
        return false;
    }
    QString strType = rx.cap(1);
    if (strType == QString("move")) {
        int x = rx.cap(2).toInt();
        int y = rx.cap(3).toInt();
        m_commondThread->sendMouseMoveEvent(x, y);
        return true;
    }

    return false;
}

/**
 * @brief NetworkCommond::cmd_changeTabPage
 * 网络键盘：T1
 * @param msg
 * @return
 */
bool NetworkCommond::cmd_TabPage(const QString &strCommond)
{
    Q_UNUSED(strCommond)

    if (SettingContent::instance()) {
        SettingContent::instance()->networkTab1();
    }
    return true;
}

bool NetworkCommond::cmd_TabPage_Prev(const QString &strCommond)
{
    Q_UNUSED(strCommond)

    if (SettingContent::instance()) {
        SettingContent::instance()->networkTab1_Prev();
    }
    return true;
}

/**
 * @brief NetworkCommond::cmd_changeFocus
 * 网络键盘：T2
 * @param strCommond
 * @return
 */
bool NetworkCommond::cmd_ChangeFocus(const QString &strCommond)
{
    NetworkResult accept = NetworkReject;

    QList<BaseWidget *> widgetList = BaseWidget::visibleList();
    for (int i = 0; i < widgetList.size(); ++i) {
        BaseWidget *widget = widgetList.at(i);
        accept = widget->dealNetworkCommond(strCommond);
        if (accept == NetworkAccept || accept == NetworkTakeOver) {
            break;
        }
    }

    if (accept == NetworkReject || accept == NetworkTakeOver) {
        m_commondThread->sendKeyEvent(KEY_TAB, KeyClick);
        accept = NetworkAccept;
    }

    return accept == NetworkAccept;
}

bool NetworkCommond::cmd_ChangeFocus_Prev(const QString &strCommond)
{
    NetworkResult accept = NetworkReject;

    QList<BaseWidget *> widgetList = BaseWidget::visibleList();
    for (int i = 0; i < widgetList.size(); ++i) {
        BaseWidget *widget = widgetList.at(i);
        accept = widget->dealNetworkCommond(strCommond);
        if (accept == NetworkAccept || accept == NetworkTakeOver) {
            break;
        }
    }

    return accept == NetworkAccept;

    //    bool accept = LiveView::instance()->networkTab_Prev();
    //    if (!accept)
    //    {
    //        accept = SettingContent::instance()->networkTab2_Prev();
    //    }

    //    return accept;
}

/**
 * @brief NetworkCommond::cmd_ptz
 * @param msg
 * @return
 */
bool NetworkCommond::cmd_Ptz(const QString &strCommond)
{
    int action = -1;
    if (strCommond == "PTZ_ZoomPlus") {
        action = PTZ_ZOOM_PLUS;
    } else if (strCommond == "PTZ_ZoomPlusStop") {
        action = PTZ_STOP_ALL;
    } else if (strCommond == "PTZ_ZoomMinus") {
        action = PTZ_ZOOM_MINUS;
    } else if (strCommond == "PTZ_ZoomMinusStop") {
        action = PTZ_STOP_ALL;
    } else if (strCommond == "PTZ_IrisPlus") {
        action = PTZ_IRIS_PLUS;
    } else if (strCommond == "PTZ_IrisPlusStop") {
        action = PTZ_STOP_ALL;
    } else if (strCommond == "PTZ_IrisMinus") {
        action = PTZ_IRIS_MINUS;
    } else if (strCommond == "PTZ_IrisMinusStop") {
        action = PTZ_STOP_ALL;
    } else if (strCommond == "PTZ_FocusPlus") {
        action = PTZ_FOCUS_PLUS;
    } else if (strCommond == "PTZ_FocusPlusStop") {
        action = PTZ_STOP_ALL;
    } else if (strCommond == "PTZ_FocusMinus") {
        action = PTZ_FOCUS_MINUS;
    } else if (strCommond == "PTZ_FocusMinusStop") {
        action = PTZ_STOP_ALL;
    } else if (strCommond == "PTZ_LightOn") {
        action = PTZ_LIGHT_ON;
    } else if (strCommond == "PTZ_LightOff") {
        action = PTZ_LIGHT_OFF;
    } else if (strCommond == "PTZ_WiperOn") {
        action = PTZ_BRUSH_ON;
    } else if (strCommond == "PTZ_WiperOff") {
        action = PTZ_BRUSH_OFF;
    } else if (strCommond == "PTZ_AutoScanOn") {
        action = PTZ_AUTO_SCAN;
    } else if (strCommond == "PTZ_AutoScanOff") {
        action = PTZ_STOP_ALL;
    } else if (strCommond == "PTZ_STOP") {
        action = PTZ_STOP_ALL;
    }
    if (action >= 0) {
        LiveView::instance()->networkPtzControl(action);
        return true;
    }

    //preset
    QRegExp rxPresetDel("PTZ_Preset_Del_(\\d+)");
    if (rxPresetDel.indexIn(strCommond) != -1) {
        int index = rxPresetDel.cap(1).toInt() - 1;
        LiveView::instance()->networkPresetControl(PTZ_PRESET_CLEAR, index);
        return true;
    }
    QRegExp rxPresetCall("PTZ_Preset_Call_(\\d+)");
    if (rxPresetCall.indexIn(strCommond) != -1) {
        int index = rxPresetCall.cap(1).toInt() - 1;
        LiveView::instance()->networkPresetControl(PTZ_PRESET_GOTO, index);
        return true;
    }
    QRegExp rxPreset("PTZ_Preset_(\\d+)");
    if (rxPreset.indexIn(strCommond) != -1) {
        int index = rxPreset.cap(1).toInt() - 1;
        LiveView::instance()->networkPresetControl(PTZ_PRESET_SET, index);
        return true;
    }

    //patrol
    QRegExp rxPatrolCall("PTZ_Patrol_Call_(\\d+)");
    if (rxPatrolCall.indexIn(strCommond) != -1) {
        int index = rxPatrolCall.cap(1).toInt() - 1;
        LiveView::instance()->networkPatrolControl(REQUEST_FLAG_PTZ_TOUR_RUN, index);
        return true;
    }
    QRegExp rxPatrolStop("PTZ_Patrol_Stop_(\\d+)");
    if (rxPatrolStop.indexIn(strCommond) != -1) {
        int index = rxPatrolStop.cap(1).toInt() - 1;
        LiveView::instance()->networkPatrolControl(REQUEST_FLAG_PTZ_TOUR_STOP, index);
        return true;
    }
    QRegExp rxPatrolDelete("PTZ_Patrol_Delete_(\\d+)");
    if (rxPatrolDelete.indexIn(strCommond) != -1) {
        int index = rxPatrolDelete.cap(1).toInt() - 1;
        LiveView::instance()->networkPatrolControl(REQUEST_FLAG_PTZ_TOUR_CLEAR, index);
        //        qDebug() << QString("LiveView::instance()->networkPatrolControl(REQUEST_FLAG_PTZ_TOUR_CLEAR, index:[%1])").arg(index);
        return true;
    }

    //pattern
    QRegExp rxPatternCall("PTZ_Pattern_Call_(\\d+)");
    if (rxPatternCall.indexIn(strCommond) != -1) {
        int index = rxPatternCall.cap(1).toInt() - 1;
        LiveView::instance()->networkPatternControl(PTZ_PATTERN_RUN, index);
        return true;
    }
    QRegExp rxPatternStop("PTZ_Pattern_Stop_(\\d+)");
    if (rxPatternStop.indexIn(strCommond) != -1) {
        int index = rxPatternStop.cap(1).toInt() - 1;
        LiveView::instance()->networkPatternControl(PTZ_STOP_ALL, index);
        return true;
    }
    QRegExp rxPatternDelete("PTZ_Pattern_Delete_(\\d+)");
    if (rxPatternDelete.indexIn(strCommond) != -1) {
        int index = rxPatternDelete.cap(1).toInt() - 1;
        LiveView::instance()->networkPatternControl(PTZ_PATTERN_DEL, index);
        return true;
    }
    return true;
}

/**
 * @brief NetworkCommond::cmd_handleDir
 * 网络键盘：摇杆
 * @param msg
 * @return
 */
bool NetworkCommond::cmd_Dir(const QString &strCommond)
{
    //Dir_Nvr_%s_%d_%d
    //Dir_Ptz_%s_%d_%d

    NetworkResult accept = NetworkReject;

    QList<BaseWidget *> widgetList = BaseWidget::visibleList();
    for (int i = 0; i < widgetList.size(); ++i) {
        BaseWidget *widget = widgetList.at(i);
        accept = widget->dealNetworkCommond(strCommond);
        if (accept == NetworkAccept || accept == NetworkTakeOver) {
            break;
        }
    }

    if (accept == NetworkReject || accept == NetworkTakeOver) {
        if (strCommond.startsWith("Dir_Nvr_Up")) {
            m_commondThread->sendKeyEvent(KEY_UP, KeyClick);
            accept = NetworkAccept;
        } else if (strCommond.startsWith("Dir_Nvr_Down")) {
            m_commondThread->sendKeyEvent(KEY_DOWN, KeyClick);
            accept = NetworkAccept;
        }
    }

    return accept == NetworkAccept;
}

/**
 * @brief NetworkCommond::cmd_Esc
 * 网络键盘：ESC
 * @param msg
 * @return
 */
bool NetworkCommond::cmd_Esc(const QString &strCommond)
{
    NetworkResult result = NetworkReject;

    QList<BaseWidget *> widgetList = BaseWidget::visibleList();
    for (int i = 0; i < widgetList.size(); ++i) {
        BaseWidget *widget = widgetList.at(i);
        result = widget->dealNetworkCommond(strCommond);
        if (result == NetworkAccept || result == NetworkTakeOver) {
            break;
        }
    }

    //
    if (result == NetworkReject || result == NetworkTakeOver) {
        m_commondThread->sendKeyEvent(KEY_ESC, KeyClick);
        result = NetworkAccept;
    }
    return result == NetworkAccept;
}

/**
 * @brief NetworkCommond::cmd_R_Click
 * @param strCommond
 * @return
 */
bool NetworkCommond::cmd_R_Click(const QString &strCommond)
{
    Q_UNUSED(strCommond)

    qDebug() << "====NetworkCommond::cmd_R_Click====";

    m_commondThread->sendMouseEvent(MouseRightButtonClick);
    return true;

    //    QWidget *widget = qApp->focusWidget();
    //    qDebug() << "----focusWidget:" << widget;
    //    if (widget)
    //    {
    //        QPoint globalPos = QCursor::pos();
    //        QPoint pos = widget->mapFromGlobal(globalPos);
    //        QMouseEvent mouseEvent(QEvent::MouseButtonPress, pos, Qt::RightButton, 0, 0);
    //        qDebug() << "----globalPos:" << globalPos;
    //        qDebug() << "----pos:" << pos;
    //        qApp->sendEvent(widget, &mouseEvent);
    //        return true;
    //    }
    //    return false;
}

bool NetworkCommond::cmd_GeneralResolver(const QString &strCommond)
{
    NetworkResult result = NetworkReject;

    QList<BaseWidget *> widgetList = BaseWidget::visibleList();
    for (int i = 0; i < widgetList.size(); ++i) {
        BaseWidget *widget = widgetList.at(i);
        result = widget->dealNetworkCommond(strCommond);
        qMsCDebug("qt_networkcommond") << widget << result;
        if (result == NetworkAccept || result == NetworkTakeOver) {
            break;
        }
    }

    return result == NetworkAccept;
}

QString NetworkCommond::currentModeName()
{
    QString name;
    switch (m_mode) {
    case MOD_WIZARD:
        name = "WIZARD";
        break;
    case MOD_LOGIN:
        name = "LOGIN";
        break;
    case MOD_LIVEVIEW:
        name = "LIVEVIEW";
        break;
    case MOD_LAYOUT:
        name = "LAYOUT";
        break;
    case MOD_PLAYBACK:
        name = "PLAYBACK";
        break;
    case MOD_CAMERA:
        name = "CAMERA";
        break;
    case MOD_RECORD:
        name = "RECORD";
        break;
    case MOD_EVENT:
        name = "EVENT";
        break;
    case MOD_SYSTEM:
        name = "SYSTEM";
        break;
    case MAD_STATUS:
        name = "STATUS";
        break;
    case MOD_SHUTDOWN:
        name = "SHUTDOWN";
        break;
    default:
        break;
    }
    return name;
}

#ifdef Keyboard_Simulation
void NetworkCommond::onTcpNewConnection()
{
    QTcpSocket *socket = m_tcpServer->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(onTcpReadyRead()));

    QString name = currentModeName();
    socket->write(QString("%1, Connected").arg(name).toUtf8());
}

void NetworkCommond::onTcpReadyRead()
{
    QTcpSocket *socket = static_cast<QTcpSocket *>(sender());
    QByteArray ba = socket->readAll();
    bool result = dealCommond(QString(ba));
    QString name = currentModeName();
    if (result) {
        socket->write(QString("%1, True").arg(name).toUtf8());
    } else {
        socket->write(QString("%1, False").arg(name).toUtf8());
    }
}
#endif
