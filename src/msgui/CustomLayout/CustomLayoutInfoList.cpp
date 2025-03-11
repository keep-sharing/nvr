#include "CustomLayoutInfoList.h"
#include <QStringList>
#include "MyDebug.h"

CustomLayoutInfoList::CustomLayoutInfoList()
{

}

CustomLayoutInfoList::~CustomLayoutInfoList()
{

}

QList<CustomLayoutInfo> CustomLayoutInfoList::infos() const
{
    return m_infoList;
}

void CustomLayoutInfoList::append(const QList<CustomLayoutInfo> &infos)
{
    m_infoList.append(infos);
}

void CustomLayoutInfoList::append(const CustomLayoutInfo &info)
{
    m_infoList.append(info);
}

void CustomLayoutInfoList::replace(const CustomLayoutInfo &info)
{
    CustomLayoutInfo &layoutInfo = find(info.key());
    if (layoutInfo.isValid()) {
        layoutInfo = info;
    } else {
        qMsWarning() << "invalid key:" << info.key();
    }
}

void CustomLayoutInfoList::remove(const CustomLayoutInfo &info)
{
    m_infoList.removeAll(info);
}

void CustomLayoutInfoList::remove(const CustomLayoutKey &key)
{
    m_infoList.removeAll(CustomLayoutInfo(key));
}

bool CustomLayoutInfoList::rename(const CustomLayoutKey &key, const QString &name)
{
    CustomLayoutInfo &info = find(key);
    if (info.isValid()) {
        info.setName(name);
        return true;
    } else {
        return false;
    }
}

bool CustomLayoutInfoList::contains(const CustomLayoutKey &key) const
{
    for (int i = 0; i < m_infoList.size(); ++i) {
        const CustomLayoutInfo &info = m_infoList.at(i);
        if (info.key() == key) {
            return true;
        }
    }
    return false;
}

int CustomLayoutInfoList::size() const
{
    return m_infoList.size();
}

const CustomLayoutInfo &CustomLayoutInfoList::at(int i) const
{
    return m_infoList.at(i);
}

const CustomLayoutInfo &CustomLayoutInfoList::find(const CustomLayoutKey &key) const
{
    for (int i = 0; i < m_infoList.size(); ++i) {
        const CustomLayoutInfo &info = m_infoList.at(i);
        if (info.key() == key) {
            return info;
        }
    }
    return m_emptyInfo;
}

CustomLayoutInfo &CustomLayoutInfoList::find(const CustomLayoutKey &key)
{
    for (int i = 0; i < m_infoList.size(); ++i) {
        const CustomLayoutInfo &info = m_infoList.at(i);
        if (info.key() == key) {
            return m_infoList[i];
        }
    }
    return m_tempInfo;
}

QStringList CustomLayoutInfoList::customLayoutNames(int screen) const
{
    QStringList names;
    for (int i = 0; i < m_infoList.size(); ++i) {
        const auto &info = m_infoList.at(i);
        if (info.screen() == screen && info.type() == CustomLayoutKey::CustomType) {
            names.append(info.name());
        }
    }
    return names;
}

CustomLayoutInfoList CustomLayoutInfoList::infos(int screen)
{
    CustomLayoutInfoList list;
    for (auto iter = m_infoList.constBegin(); iter != m_infoList.constEnd(); ++iter) {
        const CustomLayoutInfo &info = *iter;
        if (info.screen() == screen) {
            list.append(info);
        }
    }
    return list;
}

CustomLayoutInfoList CustomLayoutInfoList::takeInfos(int screen)
{
    CustomLayoutInfoList list;
    for (auto iter = m_infoList.begin(); iter != m_infoList.end();) {
        const CustomLayoutInfo &info = *iter;
        if (info.screen() == screen) {
            list.append(info);
            iter = m_infoList.erase(iter);
        } else {
            ++iter;
        }
    }
    return list;
}

void CustomLayoutInfoList::resetChannels()
{
    for (int i = 0; i < m_infoList.size(); ++i) {
        CustomLayoutInfo &info = m_infoList[i];
        info.resetChannels();
    }
}

void CustomLayoutInfoList::clear()
{
    m_infoList.clear();
}

CustomLayoutInfoList &CustomLayoutInfoList::operator=(const CustomLayoutInfoList &other)
{
    clear();
    append(other.infos());
    return *this;
}

CustomLayoutInfo &CustomLayoutInfoList::operator[](int i)
{
    return m_infoList[i];
}
