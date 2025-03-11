#ifndef CUSTOMLAYOUTDATA_H
#define CUSTOMLAYOUTDATA_H

#include "CustomLayoutInfoList.h"
#include "StreamKey.h"
#include <QObject>

extern "C" {
#include "msdb.h"
}


extern const int MaxCustomLayout;
extern const int RealCustomLayoutNameRole;

class CustomLayoutData : public QObject {
    Q_OBJECT

public:
    explicit CustomLayoutData(QObject *parent = nullptr);

    static CustomLayoutData *instance();

    QStringList customLayoutNames(int screen) const;
    QString nameFromDefaultLayoutMode(int mode) const;

    const CustomLayoutInfo &layoutInfo(const CustomLayoutKey &key) const;

    bool isChannelEnable(const CustomLayoutKey &key, int channel) const;
    bool isSingleLayout(const CustomLayoutKey &key) const;

    CustomLayoutInfoList allLayouts() const;
    void setAllLayouts(const CustomLayoutInfoList &layouts);

    void swapChannel(const CustomLayoutKey &key, int index1, int channel1, int index2, int channel2);

    void saveOldLayouts();
    void saveAllLayouts();

    void clearStreamType();
    void saveStreamType(const StreamKey &key,int streamType);
    int streamType(const StreamKey &key) const;
    int customStream(int channel);

private:
    void addLayoutMode_1();
    void addLayoutMode_4();
    void addLayoutMode_8();
    void addLayoutMode_8_1();
    void addLayoutMode_9();
    void addLayoutMode_12();
    void addLayoutMode_12_1();
    void addLayoutMode_14();
    void addLayoutMode_16();
    void addLayoutMode_25();
    void addLayoutMode_32();
    void addLayoutMode_32_2();
    void addLayoutMode_64();

signals:

private:
    static CustomLayoutData *self;

    QMap<int, mosaic> m_mosaicMap;
    QMap<StreamKey, int> m_streamMap;

    CustomLayoutInfoList m_allLayouts;
};

#endif // CUSTOMLAYOUTDATA_H
