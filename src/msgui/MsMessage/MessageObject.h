#ifndef MESSAGEOBJECT_H
#define MESSAGEOBJECT_H

#include <QPointer>
#include <QDateTime>
#include <QTimer>

class MsObject;
class MsWidget;
class MsGraphicsObject;
class MsGraphicsScene;

class MessageReceive;

class MessageObject : public QObject
{
    Q_OBJECT

public:
    enum ObjectType {
        MSQ_NULL,
        MSQ_OBJECT,
        MSQ_WIDGET,
        MSQ_GRAPHICS_OBJECT,
        MSQ_GRAPHICS_SCENE
    };

    explicit MessageObject(MsObject *obj, int message);
    explicit MessageObject(MsWidget *obj, int message);
    explicit MessageObject(MsGraphicsObject *obj, int message);
    explicit MessageObject(MsGraphicsScene *obj, int message);
    ~MessageObject() override;

    void setArgument(void *arg);
    void *argument() const;

    bool isValid() const;
    QObject *originatingObject() const;

    void clear();

    void processMessage(MessageReceive *message);

    static void showCount();

private:
    void initialize();

private slots:
    void onTimeout();

private:
    ObjectType m_type = MSQ_NULL;
    QPointer<QObject> m_pointer;
    int m_messageType = 0;
    QDateTime m_time;
    void *m_arg = nullptr;

    QTimer *m_timer = nullptr;
    int m_secs = 0;
    int m_timeout = 60;
};

QDebug operator<<(QDebug dbg, MessageObject *obj);

#endif // MESSAGEOBJECT_H
