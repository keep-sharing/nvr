#include "BaseWidget.h"
#include "MessageHelper.h"
#include "MsMessage.h"
#include "MyDebug.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QKeyEvent>
#include <QMouseEvent>

QMutex BaseWidget::s_mutex;
QList<BaseWidget *> BaseWidget::s_visibleList;

BaseWidget::BaseWidget(QWidget *parent)
    : MsWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

BaseWidget::~BaseWidget()
{
    if (s_visibleList.contains(this)) {
        s_visibleList.removeAll(this);
        qMsCDebug("qt_visible_list") << "====MyWidget::~MyWidget, Visible List, size:" << s_visibleList.size() << "====";
        for (int i = 0; i < s_visibleList.size(); ++i) {
            qMsCDebug("qt_visible_list") << "----" << s_visibleList.at(i);
        }
    }
}

BaseWidget *BaseWidget::currentWidget()
{
    if (!s_visibleList.isEmpty()) {
        return s_visibleList.first();
    } else {
        return nullptr;
    }
}

QList<BaseWidget *> BaseWidget::visibleList()
{
    return s_visibleList;
}

void BaseWidget::keyPressEvent(QKeyEvent *event)
{
    qMsCDebug("qt_widget_keypress") << "====MyWidget::keyPressEvent====";
    qMsCDebug("qt_widget_keypress") << "----this:" << this << ", event:" << event;

    switch (event->key()) {
    case Qt::Key_Escape:
        escapePressed();
        event->accept();
        break;
    case Qt::Key_Return:
        returnPressed();
        event->accept();
        break;
    default:
        break;
    }
    QWidget::keyPressEvent(event);
}

void BaseWidget::escapePressed()
{
}

void BaseWidget::returnPressed()
{
}

bool BaseWidget::isAddToVisibleList()
{
    return false;
}

NetworkResult BaseWidget::dealRockerNvr(const RockerDirection &direction)
{
    Q_UNUSED(direction)

    return NetworkReject;
}

NetworkResult BaseWidget::dealRockerPtz(const RockerDirection &direction, int hRate, int vRate)
{
    Q_UNUSED(direction)
    Q_UNUSED(hRate)
    Q_UNUSED(vRate)

    return NetworkReject;
}

NetworkResult BaseWidget::deal_Dial_Insid_Add()
{
    return NetworkReject;
}

NetworkResult BaseWidget::deal_Dial_Insid_Sub()
{
    return NetworkReject;
}

void BaseWidget::sendMessage(int type, const void *data, int size)
{

    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}

void BaseWidget::sendMessageOnly(int type, const void *data, int size)
{
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}

void BaseWidget::loadStylesheet(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        setStyleSheet(stream.readAll());
    } else {
        qMsCritical() << "load stylesheet failed:" << filePath << "," << file.errorString();
    }
    file.close();
}

void BaseWidget::focusPreviousChild()
{
    QWidget::focusPreviousChild();
}

void BaseWidget::focusNextChild()
{
    QWidget::focusNextChild();
}

NetworkResult BaseWidget::dealNetworkCommond(const QString &commond)
{
    NetworkResult result = NetworkReject;
    if (commond.startsWith("ChangeFocus_")) {
        focusNextChild();
        result = NetworkAccept;
    } else if (commond.startsWith("ChangeFocus_Prev")) {
        focusPreviousChild();
        result = NetworkAccept;
    } else if (commond.startsWith("Esc")) {
        escapePressed();
        result = NetworkAccept;
    } else if (commond.startsWith("Dir_")) {
        //Dir_Nvr_%s_%d_%d
        //Dir_Ptz_%s_%d_%d

        QRegExp rx("Dir_(.+)_(.+)_(.+)_(.+)");
        if (rx.indexIn(commond) == -1) {
            result = NetworkReject;
        } else {
            //Nvr, Ptz
            QString strType = rx.cap(1);
            const QString strDirection = rx.cap(2);
            QString strHrate = rx.cap(3);
            QString strVrate = rx.cap(4);
            int hRate = strHrate.toInt();
            int vRate = strVrate.toInt();
            RockerDirection direction = RockerNone;
            if (strDirection == "Up") {
                direction = RockerUp;
            } else if (strDirection == "Down") {
                direction = RockerDown;
            } else if (strDirection == "Left") {
                direction = RockerLeft;
            } else if (strDirection == "Right") {
                direction = RockerRight;
            } else if (strDirection == "UpLeft") {
                direction = RockerUpLeft;
            } else if (strDirection == "UpRight") {
                direction = RockerUpRight;
            } else if (strDirection == "DownLeft") {
                direction = RockerDownLeft;
            } else if (strDirection == "DownRight") {
                direction = RockerDownRight;
            } else if (strDirection == "STOP") {
                direction = RockerStop;
            }
            //
            if (strType == "Nvr") {
                result = dealRockerNvr(direction);
            } else if (strType == "Ptz") {
                result = dealRockerPtz(direction, hRate, vRate);
            }
        }
    } else if (commond.startsWith("Dial_Insid_Add")) {
        result = deal_Dial_Insid_Add();
    } else if (commond.startsWith("Dial_Insid_Sub")) {
        result = deal_Dial_Insid_Sub();
    }
    return result;
}

void BaseWidget::showEvent(QShowEvent *event)
{
    if (isAddToVisibleList()) {
        s_visibleList.prepend(this);
        qMsCDebug("qt_visible_list") << "====MyWidget::showEvent, Visible List, size:" << s_visibleList.size() << "====";
        for (int i = 0; i < s_visibleList.size(); ++i) {
            qMsCDebug("qt_visible_list") << "----" << s_visibleList.at(i);
        }
    }
    setFocus();
    QWidget::showEvent(event);
}

void BaseWidget::hideEvent(QHideEvent *event)
{
    if (s_visibleList.contains(this)) {
        s_visibleList.removeAll(this);
        qMsCDebug("qt_visible_list") << "====MyWidget::hideEvent, Visible List, size:" << s_visibleList.size() << "====";
        for (int i = 0; i < s_visibleList.size(); ++i) {
            qMsCDebug("qt_visible_list") << "----" << s_visibleList.at(i);
        }
    }
    QWidget::hideEvent(event);
}
