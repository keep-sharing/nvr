#include "InvokeInfo.h"

InvokeInfo::InvokeInfo(QObject *obj, const QString &member)
    : m_obj(obj)
    , m_member(member)
{
}

QObject *InvokeInfo::obj() const
{
    return m_obj;
}

void InvokeInfo::setObj(QObject *newObj)
{
    m_obj = newObj;
}

const QString &InvokeInfo::member() const
{
    return m_member;
}

void InvokeInfo::setMember(const QString &newMember)
{
    m_member = newMember;
}
