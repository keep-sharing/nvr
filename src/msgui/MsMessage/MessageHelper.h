#ifndef MESSAGEHELPER_H
#define MESSAGEHELPER_H

#include <QHash>
#include <QMap>
#include <QMutex>
#include <QObject>

#define gMessageHelper MessageHelper::instance()

class MessageHelper : public QObject {
    Q_OBJECT

public:
    explicit MessageHelper(QObject *parent = nullptr);

    static MessageHelper &instance();
    void initialize();

    bool isNeedResponse(int type) const;
    QString messageTypeString(int type) const;

    //
    void requestCountAdd(int type);
    void responseCountAdd(int type);
    void makeMessageCountReport();

signals:

private:
    QHash<int, QString> m_messageTypeHash;

    //消息调用统计
    QMutex m_mutexCount;
    QMap<int, int> m_requestMap;
    QMap<int, int> m_responseMap;
    QMap<int, quint32> m_requestCountMap;
    QMap<int, quint32> m_responseCountMap;
};

#endif // MESSAGEHELPER_H
