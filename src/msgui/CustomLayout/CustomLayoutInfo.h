#ifndef CUSTOMLAYOUTINFO_H
#define CUSTOMLAYOUTINFO_H

#include <QMap>
#include <QRectF>
#include "CustomLayoutKey.h"
#include "VideoPosition.h"

class CustomLayoutInfo
{
public:
    explicit CustomLayoutInfo();
    explicit CustomLayoutInfo(const QString &name, int screen, CustomLayoutKey::LayoutType type);
    explicit CustomLayoutInfo(const QString &name, int screen, int baseRow, int baseColumn);
    explicit CustomLayoutInfo(const CustomLayoutKey &key);

    void clear();

    bool isValid() const;
    CustomLayoutKey key() const;
    void addDefaultLayout(const QString &name, int screen, int baseRow, int baseColumn);
    void resetChannels();

    QString name() const;
    void setName(const QString &name);
    int screen() const;
    void setScreen(int screen);
    CustomLayoutKey::LayoutType type() const;
    void setType(CustomLayoutKey::LayoutType type);
    int baseRow() const;
    void setBaseRow(int baseRow);
    int baseColumn() const;
    void setBaseColumn(int baseColumn);
    QMap<VideoPosition, QRectF> positions() const;
    void setPositions(const QMap<VideoPosition, QRectF> &positions);
    void insertPosition(const VideoPosition &position, const QRectF &rc);
    QMap<int, int> channels() const;
    void setChannels(const QMap<int, int> &channels);
    int channel(int index) const;
    void insertChannel(int index, int channel);
    void updateChannel(int index, int channel);
    bool isChannelEnable(int channel) const;

    bool isSingleLayout() const;

    int positionCount() const;
    int pageCount() const;
    QMap<int, int> pageMap() const;
    int pageOfChannel(int channel) const;

    int defaultLayoutMode() const;

    void swapChannel(int index1, int channel1, int index2, int channel2);

    bool operator ==(const CustomLayoutInfo &other) const;

private:
    QString m_name;
    int m_screen = 0;
    CustomLayoutKey::LayoutType m_type = CustomLayoutKey::CustomType;
    int m_baseRow = 5;
    int m_baseColumn = 5;
    QMap<VideoPosition, QRectF> m_positions;
    //index, channel
    QMap<int, int> m_channels;
};

#endif // CUSTOMLAYOUT_H

