#ifndef CUSTOMLAYOUTINFOLIST_H
#define CUSTOMLAYOUTINFOLIST_H

#include "CustomLayoutInfo.h"

class CustomLayoutInfoList
{
public:
    CustomLayoutInfoList();
    virtual ~CustomLayoutInfoList();

    QList<CustomLayoutInfo> infos() const;
    void append(const QList<CustomLayoutInfo> &infos);

    void append(const CustomLayoutInfo &info);
    void replace(const CustomLayoutInfo &info);

    void remove(const CustomLayoutInfo &info);
    void remove(const CustomLayoutKey &key);

    bool rename(const CustomLayoutKey &key, const QString &name);

    bool contains(const CustomLayoutKey &key) const;

    int size() const;
    const CustomLayoutInfo &at(int i) const;

    const CustomLayoutInfo &find(const CustomLayoutKey &key) const;
    CustomLayoutInfo &find(const CustomLayoutKey &key);

    QStringList customLayoutNames(int screen) const;

    CustomLayoutInfoList infos(int screen);
    CustomLayoutInfoList takeInfos(int screen);

    void resetChannels();
    void clear();

    CustomLayoutInfoList &operator=(const CustomLayoutInfoList &other);

    CustomLayoutInfo &operator[](int i);

private:
    QList<CustomLayoutInfo> m_infoList;
    const CustomLayoutInfo m_emptyInfo;
    CustomLayoutInfo m_tempInfo;
};

#endif // CUSTOMLAYOUTINFOLIST_H
