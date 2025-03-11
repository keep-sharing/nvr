#ifndef MYDEBUG_H
#define MYDEBUG_H

#include <QDateTime>
#include <QHash>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtDebug>
#include "MemoryAnalysis.h"
#include "MyTcpServer.h"

class QLocalServer;
class QLocalSocket;
class QSettings;

#define qMsDebug() (qDebug() << qPrintable(QString("[GUI][%1][Debug][%2 %3]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")).arg(__LINE__).arg(__PRETTY_FUNCTION__)))
#define qMsWarning() (qWarning() << qPrintable(QString("[GUI][%1][Warning][%2 %3]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")).arg(__LINE__).arg(__PRETTY_FUNCTION__)))
#define qMsCritical() (qCritical() << qPrintable(QString("[GUI][%1][Critical][%2 %3]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")).arg(__LINE__).arg(__PRETTY_FUNCTION__)))
#define qMsCDebug(text) (qDebug() << qPrintable(QString("[%1][%2][%3 %4]").arg(text).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")).arg(__LINE__).arg(__PRETTY_FUNCTION__)))
#define yCDebug(text) (qDebug() << qPrintable(QString("[%1][%2]").arg(text).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))))
#define yDebug() (qDebug() << qPrintable(QString("[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))))
#define yWarning() (qWarning() << qPrintable(QString("[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))))
#define yCritical() (qCritical() << qPrintable(QString("[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))))
#define QMS_ASSERT(cond) ((!(cond)) ? qt_assert(#cond, __FILE__, __LINE__) : qt_noop())

#define gDebug MyDebug::instance()

#define STRUCT(type, pInstance, ...) \
    {                                \
        qDebug() << #type << " "     \
                 << #pInstance       \
                 << endl;            \
        type *pStr = pInstance;      \
        __VA_ARGS__                  \
    }
#define FIELD(type, name)          \
    {                              \
        qDebug() << #type << " "   \
                 << #name << " = " \
                 << pStr->name;     \
    }

class MyDebug : public QObject {
    Q_OBJECT
public:
    explicit MyDebug(QObject *parent = nullptr);

    static MyDebug &instance();

    void installMemoryHandler();

    void installMessageHandler();
    void showMessage(const QString &text);

    bool checkDebugCategory(const QString &category);

    void setDebugCategory(const QString &category, const QString &data = QString());
    void clearDebugCategory(const QString &category);
    void clearAllDebugCategory();

    //set
    bool checkCategorySet(const QString &key, const QString &value) const;
    bool hasCategorySet(const QString &key);
    QString categorySet(const QString &key) const;

private:
    void dealShowMessage(const QString &data);
    void dealHideMessage(const QString &data);

    void dealOpenMessage(const QString &data);
    void dealCloseMessage(const QString &data);

    void dealSetMessage(const QString &data);
    void dealGetMessage(const QString &data);

    void dealScriptMessage(const QString &data);

    void dealDiskMessage(const QString &data);

signals:
    void message(const QString &text);

private slots:
    void initializeData();

    void onNewConnection();
    void onReadyRead();

private:
    static QMutex *s_mutex;

    QLocalServer *m_localServer = nullptr;

    QSettings *m_settings = nullptr;
    QHash<QString, QString> m_categoryHash;
    QHash<QString, QString> m_categorySet;

    //
    MyTcpServer *m_tcpServer = nullptr;

};

#endif // MYDEBUG_H
