#include "CustomLayoutInfo.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include <qmath.h>

extern "C" {
#include "msdb.h"
}

CustomLayoutInfo::CustomLayoutInfo()
{
}

CustomLayoutInfo::CustomLayoutInfo(const QString &name, int screen, CustomLayoutKey::LayoutType type)
    : m_name(name)
    , m_screen(screen)
    , m_type(type)
{
}

CustomLayoutInfo::CustomLayoutInfo(const QString &name, int screen, int baseRow, int baseColumn)
    : m_name(name)
    , m_screen(screen)
    , m_baseRow(baseRow)
    , m_baseColumn(baseColumn)
{
}

CustomLayoutInfo::CustomLayoutInfo(const CustomLayoutKey &key)
    : m_name(key.name())
    , m_screen(key.screen())
    , m_type(key.type())
{
}

void CustomLayoutInfo::clear()
{
    m_name.clear();
    m_positions.clear();
    m_channels.clear();
}

bool CustomLayoutInfo::isValid() const
{
    bool valid = true;
    if (m_name.isEmpty()) {
        return false;
    }
    return valid;
}

CustomLayoutKey CustomLayoutInfo::key() const
{
    return CustomLayoutKey(m_name, m_screen, m_type);
}

void CustomLayoutInfo::addDefaultLayout(const QString &name, int screen, int baseRow, int baseColumn)
{
    m_positions.clear();
    m_name = name;
    m_screen = screen;
    m_type = CustomLayoutKey::CustomType;
    m_baseRow = baseRow;
    m_baseColumn = baseColumn;
    for (int r = 0; r < baseRow; ++r) {
        for (int c = 0; c < baseColumn; ++c) {
            VideoPosition p;
            p.row = r;
            p.column = c;
            p.rowSpan = 1;
            p.columnSpan = 1;
            m_positions.insert(p, QRectF());
        }
    }
    for (int i = 0; i < MAX_CAMERA; ++i) {
        m_channels.insert(i, i);
    }
}

void CustomLayoutInfo::resetChannels()
{
    for (int i = 0; i < MAX_CAMERA; ++i) {
        insertChannel(i, i);
    }
}

QString CustomLayoutInfo::name() const
{
    return m_name;
}

void CustomLayoutInfo::setName(const QString &name)
{
    m_name = name;
}

int CustomLayoutInfo::screen() const
{
    return m_screen;
}

void CustomLayoutInfo::setScreen(int screen)
{
    m_screen = screen;
}

CustomLayoutKey::LayoutType CustomLayoutInfo::type() const
{
    return m_type;
}

void CustomLayoutInfo::setType(CustomLayoutKey::LayoutType type)
{
    m_type = type;
}

int CustomLayoutInfo::baseRow() const
{
    return m_baseRow;
}

void CustomLayoutInfo::setBaseRow(int baseRow)
{
    m_baseRow = baseRow;
}

int CustomLayoutInfo::baseColumn() const
{
    return m_baseColumn;
}

void CustomLayoutInfo::setBaseColumn(int baseColumn)
{
    m_baseColumn = baseColumn;
}

QMap<VideoPosition, QRectF> CustomLayoutInfo::positions() const
{
    return m_positions;
}

void CustomLayoutInfo::setPositions(const QMap<VideoPosition, QRectF> &positions)
{
    m_positions = positions;
}

void CustomLayoutInfo::insertPosition(const VideoPosition &position, const QRectF &rc)
{
    m_positions.insert(position, rc);
}

QMap<int, int> CustomLayoutInfo::channels() const
{
    return m_channels;
}

void CustomLayoutInfo::setChannels(const QMap<int, int> &channels)
{
    m_channels = channels;
}

int CustomLayoutInfo::channel(int index) const
{
    if (index >= qMsNvr->maxChannel()) {
        return -1;
    }
    return m_channels.value(index, -1);
}

void CustomLayoutInfo::insertChannel(int index, int channel)
{
    m_channels.insert(index, channel);
}

void CustomLayoutInfo::updateChannel(int index, int channel)
{
    for (int i = 0; i < MAX_CAMERA; ++i) {
        if (i == index) {
            m_channels[i] = channel;
        } else if (m_channels.value(i) == channel && channel != -1) {
            m_channels[i] = -1;
        }
    }
}

bool CustomLayoutInfo::isChannelEnable(int channel) const
{
    for (auto iter = m_channels.constBegin(); iter != m_channels.constEnd(); ++iter) {
        if (iter.value() == channel) {
            return true;
        }
    }
    return false;
}

bool CustomLayoutInfo::isSingleLayout() const
{
    return m_positions.size() == 1;
}

int CustomLayoutInfo::positionCount() const
{
    return m_positions.size();
}

int CustomLayoutInfo::pageCount() const
{
    if (m_positions.isEmpty()) {
        qMsWarning() << "positions is empty.";
        return 0;
    }
    return qCeil((qreal)qMsNvr->maxChannel() / m_positions.size());
}

QMap<int, int> CustomLayoutInfo::pageMap() const
{
    const QMap<int, camera> &cameraMap = qMsNvr->cameraMap();
    int maxChannel = qMsNvr->maxChannel();

    QMap<int, int> map;
    int count = pageCount();
    for (int page = 0; page < count; ++page) {
        int indexBegin = page * m_positions.size();
        int indexEnd = qMin((page + 1) * m_positions.size() - 1, maxChannel - 1);
        for (int i = indexBegin; i <= indexEnd; ++i) {
            int ch = channel(i);
            if (cameraMap.contains(ch)) {
                const camera &c = cameraMap.value(ch);
                if (c.enable && c.id < maxChannel) {
                    map.insert(map.size(), page);
                    break;
                }
            }
        }
    }
    return map;
}

int CustomLayoutInfo::pageOfChannel(int channel) const
{
    int count = positionCount();
    QMap<int, int> pages = pageMap();

    int realPage = -1;
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const int &c = m_channels.value(i, -1);
        if (c == channel) {
            realPage = i / count;
            break;
        }
    }

    //
    int page = -1;
    for (auto iter = pages.constBegin(); iter != pages.constEnd(); ++iter) {
        if (iter.value() == realPage) {
            page = iter.key();
            break;
        }
    }

    return page;
}

int CustomLayoutInfo::defaultLayoutMode() const
{
    if (type() != CustomLayoutKey::DefaultType) {
        return -1;
    }

    int mode = -1;
    if (m_name == QString("LAYOUTMODE_1")) {
        mode = LAYOUTMODE_1;
    } else if (m_name == QString("LAYOUTMODE_4")) {
        mode = LAYOUTMODE_4;
    } else if (m_name == QString("LAYOUTMODE_8")) {
        mode = LAYOUTMODE_8;
    } else if (m_name == QString("LAYOUTMODE_8_1")) {
        mode = LAYOUTMODE_8_1;
    } else if (m_name == QString("LAYOUTMODE_9")) {
        mode = LAYOUTMODE_9;
    } else if (m_name == QString("LAYOUTMODE_12")) {
        mode = LAYOUTMODE_12;
    } else if (m_name == QString("LAYOUTMODE_12_1")) {
        mode = LAYOUTMODE_12_1;
    } else if (m_name == QString("LAYOUTMODE_14")) {
        mode = LAYOUTMODE_14;
    } else if (m_name == QString("LAYOUTMODE_16")) {
        mode = LAYOUTMODE_16;
    } else if (m_name == QString("LAYOUTMODE_25")) {
        mode = LAYOUTMODE_25;
    } else if (m_name == QString("LAYOUTMODE_32")) {
        mode = LAYOUTMODE_32;
    } else if (m_name == QString("LAYOUTMODE_36")) {
        mode = LAYOUTMODE_36;
    } else if (m_name == QString("LAYOUTMODE_32_2")) {
        mode = LAYOUTMODE_32_2;
    }
    else if (m_name == QString("LAYOUTMODE_64")) {
      mode = LAYOUTMODE_64;
    }
    return mode;
}

void CustomLayoutInfo::swapChannel(int index1, int channel1, int index2, int channel2)
{
    m_channels.insert(index1, channel2);
    m_channels.insert(index2, channel1);
}

bool CustomLayoutInfo::operator==(const CustomLayoutInfo &other) const
{
    return name() == other.name() && screen() == other.screen() && type() == other.type();
}
