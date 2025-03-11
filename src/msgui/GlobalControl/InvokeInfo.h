#ifndef INVOKEINFO_H
#define INVOKEINFO_H

#include <QObject>
#include <QString>

class InvokeInfo
{
public:
    InvokeInfo(QObject *obj, const QString &member);

    QObject *obj() const;
    void setObj(QObject *newObj);

    const QString &member() const;
    void setMember(const QString &newMember);

private:
    QObject *m_obj = nullptr;
    QString m_member;
};

#endif // INVOKEINFO_H
