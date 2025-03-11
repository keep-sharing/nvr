#ifndef MESSAGEFILTER_H
#define MESSAGEFILTER_H

#include <QObject>
#include <QPointer>
#include <QMap>

class MsObject;
class MsWidget;
class MsGraphicsObject;
class MsGraphicsScene;

class MessageReceive;

#define gMessageFilter MessageFilter::instance()

class MessageFilter : public QObject {
    Q_OBJECT

    struct ObjInfo {
        ObjInfo(int t, QObject *p)
        {
            objType = t;
            objPointer = p;
        }
        int objType = 0;
        QPointer<QObject> objPointer;
    };

public:
    explicit MessageFilter(QObject *parent = nullptr);

    static MessageFilter &instance();

    void processEventFilter(MessageReceive *message);

    /*************************************************
     * 正常消息是谁发送谁接收，中心主动发起的消息无法确定接收者
     * 需要接收的地方安装一个过滤器
     * 过滤器接收消息的优先级是最低的
    *************************************************/
    void installMessageFilter(int type, MsObject *obj);
    void installMessageFilter(int type, MsWidget *obj);
    void installMessageFilter(int type, MsGraphicsObject *obj);
    void installMessageFilter(int type, MsGraphicsScene *obj);

    /*************************************************
     * 像Object::connect一样，对象销毁后会自动移除过滤器
     * 需要的话也可以手动移除
    *************************************************/
    void removeMessageFilter(int type, QObject *obj);

signals:

private:
    QMap<int, QList<ObjInfo>> m_filterMap;
};

#endif // MESSAGEFILTER_H
