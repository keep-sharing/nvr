#include "MyDebug.h"
#include "AutoLogoutTip.h"
#include "BasePlayback.h"
#include "BreakpadHandler.h"
#include "MessageHelper.h"
#include "MessageObject.h"
#include "MsWaitting.h"
#include "RunnableDiskRawDataImport.h"
#include "Script.h"
#include "SettingTimeoutTip.h"
#include "mainwindow.h"
#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMutex>
#include <QRegExp>
#include <QSettings>
#include <QThreadPool>
#include "tsar_socket.h"
#include <QBuffer>

QMutex *MyDebug::s_mutex = new QMutex(QMutex::Recursive);

void myMessageOutput(QtMsgType type, const char *msg)
{
    QString text(msg);

    if (text.startsWith("[")) {
        //qMsCDebug
        QString category;
        if (text.startsWith("[qt")) {
            QRegExp rx(R"(\[(qt_.+)\])");
            rx.setMinimal(true);
            if (rx.indexIn(text) != -1) {
                category = rx.cap(1);
            }
            //30:黑，31:红，32:绿, 33:黄, 34:蓝, 35:紫，36:青，37:白
            if (gDebug.checkDebugCategory(category)) {
                if (category == "qt_message" || category == "qt_mouse") {
                    ms_log(TSAR_INFO, "%s ", text.toStdString().c_str());
                }
                if (category == "qt_message") {
                    fprintf(stderr, "\033[36m%s\033[0m\n", text.toStdString().c_str());
                } else {
                    switch (type) {
                    case QtDebugMsg:
                        fprintf(stderr, "\033[33m%s\033[0m\n", text.toStdString().c_str());
                        break;
                    case QtWarningMsg:
                        fprintf(stderr, "\033[31m%s\033[0m\n", text.toStdString().c_str());
                        break;
                    case QtCriticalMsg:
                        fprintf(stderr, "\033[35m%s\033[0m\n", text.toStdString().c_str());
                        break;
                    case QtFatalMsg:
                        fprintf(stderr, "\033[35m%s\033[0m\n", text.toStdString().c_str());
                        break;
                    }
                }
                gDebug.showMessage(text);
            }
        } else {
            //qMsDebug
            //31:红，32:绿, 33:黄, 34:蓝, 35: 紫
            switch (type) {
            case QtDebugMsg:
                fprintf(stderr, "\033[32m%s\033[0m\n", text.toStdString().c_str());
                break;
            case QtWarningMsg:
                fprintf(stderr, "\033[31m%s\033[0m\n", text.toStdString().c_str());
                break;
            case QtCriticalMsg:
                fprintf(stderr, "\033[35m%s\033[0m\n", text.toStdString().c_str());
                break;
            case QtFatalMsg:
                fprintf(stderr, "\033[35m%s\033[0m\n", text.toStdString().c_str());
                break;
            }
            gDebug.showMessage(text);
        }
    } else {
        //兼容qDebug
        text = QString("[GUI][%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
        switch (type) {
        case QtDebugMsg:
            text.append(QString("qDebug: %1").arg(msg));
            fprintf(stderr, "\033[32m%s\033[0m\n", text.toStdString().c_str());
            break;
        case QtWarningMsg:
            text.append(QString("qWarning: %1").arg(msg));
            fprintf(stderr, "\033[31m%s\033[0m\n", text.toStdString().c_str());
            break;
        case QtCriticalMsg:
            text.append(QString("qCritical: %1").arg(msg));
            fprintf(stderr, "\033[35m%s\033[0m\n", text.toStdString().c_str());
            break;
        case QtFatalMsg:
            text.append(QString("qFatal: %1").arg(msg));
            fprintf(stderr, "\033[35m%s\033[0m\n", text.toStdString().c_str());
            break;
        }
        gDebug.showMessage(text);
    }
}

void myMemoryHandler(QT_MEM_TYPE_E type, void *mem_old, void *mem_new, int size, const char *file, int line)
{
    switch (type) {
    case QT_MEM_ADD:
        gMemoryAnalysis.addMemoryInfo(mem_new, size, file, line);
        break;
    case QT_MEM_DEL:
        gMemoryAnalysis.removeMemoryInfo(mem_new, file, line);
        break;
    case QT_MEM_REP:
        gMemoryAnalysis.replaceMemoryInfo(mem_old, mem_new, size, file, line);
        break;
    default:
        break;
    }
}

MyDebug::MyDebug(QObject *parent)
    : QObject(parent)
{
    m_settings = new QSettings("/mnt/nand/qmsdebug.conf", QSettings::IniFormat, this);
    QStringList keys = m_settings->allKeys();
    foreach (const QString &key, keys) {
        setDebugCategory(key);
    }

    //
#if 1
    setDebugCategory("qt_message");
    setDebugCategory("qt_mouse");
#endif

    //
    QMetaObject::invokeMethod(this, "initializeData", Qt::QueuedConnection);
}

MyDebug &MyDebug::instance()
{
    static MyDebug self;
    return self;
}

void MyDebug::installMemoryHandler()
{
    set_qt_callback_mem(myMemoryHandler);
}

void MyDebug::installMessageHandler()
{
    qInstallMsgHandler(myMessageOutput);
}

void MyDebug::showMessage(const QString &text)
{
    emit message(text);
}

bool MyDebug::checkDebugCategory(const QString &category)
{
    QMutexLocker locker(s_mutex);

    if (category.isEmpty()) {
        return true;
    }

    if (m_categoryHash.contains(category)) {
        return true;
    } else {
        return false;
    }
}

void MyDebug::setDebugCategory(const QString &category, const QString &data)
{
    QMutexLocker locker(s_mutex);
    m_categoryHash.insert(category, data);

    m_settings->setValue(category, data);
}

void MyDebug::clearDebugCategory(const QString &category)
{
    QMutexLocker locker(s_mutex);
    m_categoryHash.remove(category);

    m_settings->remove(category);
}

void MyDebug::clearAllDebugCategory()
{
    QMutexLocker locker(s_mutex);
    m_categoryHash.clear();

    m_settings->clear();
}

bool MyDebug::checkCategorySet(const QString &key, const QString &value) const
{
    return m_categorySet.value(key) == value;
}

bool MyDebug::hasCategorySet(const QString &key)
{
    return m_categorySet.contains(key);
}

QString MyDebug::categorySet(const QString &key) const
{
    return m_categorySet.value(key);
}

void MyDebug::dealShowMessage(const QString &data)
{
    if (data == QString("qt_all")) {

    } else {
        setDebugCategory(data);
    }
}

void MyDebug::dealHideMessage(const QString &data)
{
    if (data == QString("qt_all")) {
        clearAllDebugCategory();
    } else {
        clearDebugCategory(data);
    }
}

void MyDebug::dealOpenMessage(const QString &data)
{
    if (data == QString("auto_logout_tip")) {
        AutoLogoutTip *tip = AutoLogoutTip::instance();
        if (tip) {
            tip->show();
            tip->move(0, 100);
        }
    } else if (data == QString("setting_timeout_tip")) {
        SettingTimeoutTip *tip = SettingTimeoutTip::instance();
        if (tip) {
            tip->show();
            tip->move(0, 200);
        }
    }
}

void MyDebug::dealCloseMessage(const QString &data)
{
    if (data == QString("auto_logout_tip")) {
        AutoLogoutTip *tip = AutoLogoutTip::instance();
        if (tip) {
            tip->hide();
        }
    } else if (data == QString("setting_timeout_tip")) {
        SettingTimeoutTip *tip = SettingTimeoutTip::instance();
        if (tip) {
            tip->hide();
        }
    }
}

void MyDebug::dealSetMessage(const QString &data)
{
    if (data == QString("clear_all")) {
        m_categorySet.clear();
    } else if (data == QString("dump")) {
        BreakpadHandler::instance().writeDump();
    } else {
        QStringList list = data.split(" ");
        if (list.size() == 2) {
            m_categorySet.insert(list.at(0), list.at(1));
        }
    }
}

void MyDebug::dealGetMessage(const QString &data)
{
    if (data == QString("manual_close_wait")) {
        qMsWarning() << "manual close wait";
        //MsWaitting::instance()->//closeWait();
    } else if (data == QString("manual_close_eventloop")) {
        qMsWarning() << "manual close eventloop";
        BasePlayback::exitWait();
    } else if (data == QString("message_report")) {
        gMessageHelper.makeMessageCountReport();
    } else if (data == QString("message_count")) {
        MessageObject::showCount();
    }
}

void MyDebug::dealScriptMessage(const QString &data)
{
    if (data == QString("open")) {
        if (!Script::instance()) {
            Script *script = new Script();
            script->show();
            script->move(0, 0);
        }
    } else if (data == QString("close")) {
        if (Script::instance()) {
            delete Script::instance();
        }
    } else if (data == QString("run")) {
        if (Script::instance()) {
            Script::instance()->scriptRun();
        }
    } else if (data == QString("stop")) {
        if (Script::instance()) {
            Script::instance()->scriptStop();
        }
    }
}

/**
 * @brief MyDebug::dealDiskMessage
 * @param data
 * web导出：192.168.63.140:5678/backup_disk_raw_data?port=8&offset=181703607808
 * u盘导入：msqdebug disk import 8 /media/usb1_1/disk_8_151638836736.raw
 * 重建索引：mscli disk msfs restore 8
 */
void MyDebug::dealDiskMessage(const QString &data)
{
    QStringList strList = data.split(" ", QString::SkipEmptyParts);
    if (strList.size() != 3) {
        qMsWarning() << "error disk message:" << data;
        return;
    }
    int port = strList.at(1).toInt();
    QString filePath = strList.at(2);
    QThreadPool::globalInstance()->start(new RunnableDiskRawDataImport(port, filePath));
}

void MyDebug::initializeData()
{
    //
    m_localServer = new QLocalServer(this);
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    m_localServer->removeServer("msqdebug");
    bool ok = m_localServer->listen("msqdebug");
    if (!ok) {
        qMsWarning() << "LocalServer listen error:" << m_localServer->errorString();
    }
    //
    m_tcpServer = new MyTcpServer(this);
    ok = m_tcpServer->listen(QHostAddress::Any, 5678);
    if (!ok) {
        qMsWarning() << "TcpServer listen error:" << m_tcpServer->errorString();
    }
}

void MyDebug::onNewConnection()
{
    QLocalSocket *socket = m_localServer->nextPendingConnection();
    if (socket) {
        connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    }
}

void MyDebug::onReadyRead()
{
    QLocalSocket *socket = static_cast<QLocalSocket *>(sender());
    QByteArray ba = socket->readAll();

    QDataStream stream(&ba, QIODevice::ReadOnly);
    QString type;
    stream >> type;
    QString data;
    while (!stream.atEnd()) {
        QString str;
        stream >> str;
        data.append(str);
        data.append(" ");
    }
    if (data.endsWith(" ")) {
        data.chop(1);
    }

    qMsDebug() << "[msqdebug]" << type << data;

    if (type == QString("show")) {
        dealShowMessage(data);
    } else if (type == QString("hide")) {
        dealHideMessage(data);
    } else if (type == QString("open")) {
        dealOpenMessage(data);
    } else if (type == QString("close")) {
        dealCloseMessage(data);
    } else if (type == QString("set")) {
        dealSetMessage(data);
    } else if (type == QString("get")) {
        dealGetMessage(data);
    } else if (type == QString("script")) {
        dealScriptMessage(data);
    } else if (type == QString("disk")) {
        dealDiskMessage(data);
    }
}
