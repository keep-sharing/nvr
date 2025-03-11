#include "CustomLayoutData.h"
#include "MyDebug.h"
#include "StreamKey.h"
#include "mainwindow.h"
#include "msstd.h"
#include "qglobal.h"

const int MaxAllLayout = 100;
const int MaxCustomLayout = 10;
extern const int RealCustomLayoutNameRole = Qt::UserRole + 1;

CustomLayoutData *CustomLayoutData::self = nullptr;

CustomLayoutData::CustomLayoutData(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<CustomLayoutKey>("CustomLayoutKey");

    custom_mosaic mosaics[MaxAllLayout];
    memset(mosaics, 0, sizeof(mosaics));
    int count = 0;
    read_custom_mosaics_64Screen(SQLITE_FILE_NAME, mosaics, &count);
    for (int i = 0; i < MaxAllLayout; ++i) {
        const custom_mosaic &m = mosaics[i];
        if (QString(m.name).isEmpty()) {
            break;
        }
        CustomLayoutInfo info;
        info.setName(m.name);
        info.setScreen(m.screen);
        info.setType(static_cast<CustomLayoutKey::LayoutType>(m.type));
        info.setBaseRow(m.baseRow);
        info.setBaseColumn(m.baseColumn);
        for (int j = 0; j < MAX_CAMERA; ++j) {
            VideoPosition p;
            p.index = m.positions[j].index;
            p.row = m.positions[j].row;
            p.column = m.positions[j].column;
            p.rowSpan = m.positions[j].rowSpan;
            p.columnSpan = m.positions[j].columnSpan;
            if (p.rowSpan > 0 && p.columnSpan > 0) {
                info.insertPosition(p, QRectF());
            }
            info.insertChannel(j, m.channels[j]);
        }
        m_allLayouts.append(info);
    }

    mosaic mosaicArray[LAYOUT_MODE_MAX];
    memset(mosaicArray, 0, sizeof(mosaic) * LAYOUT_MODE_MAX);
    int mosaicCount = 0;
    read_mosaics(SQLITE_FILE_NAME, mosaicArray, &mosaicCount);
    for (int i = 0; i < mosaicCount; ++i) {
        mosaic &m = mosaicArray[i];
        m_mosaicMap.insert(m.layoutmode, m);
    }
    //
    addLayoutMode_1();
    addLayoutMode_4();
    addLayoutMode_8();
    addLayoutMode_8_1();
    addLayoutMode_9();
    addLayoutMode_12();
    addLayoutMode_12_1();
    addLayoutMode_14();
    addLayoutMode_16();
    addLayoutMode_25();
    addLayoutMode_32();
    addLayoutMode_32_2();
    addLayoutMode_64();
}

CustomLayoutData *CustomLayoutData::instance()
{
    if (!self) {
        self = new CustomLayoutData(MainWindow::instance());
    }
    return self;
}

QStringList CustomLayoutData::customLayoutNames(int screen) const
{
    return m_allLayouts.customLayoutNames(screen);
}

QString CustomLayoutData::nameFromDefaultLayoutMode(int mode) const
{
    QString name;
    switch (mode) {
    case LAYOUTMODE_1:
        name = "LAYOUTMODE_1";
        break;
    case LAYOUTMODE_4:
        name = "LAYOUTMODE_4";
        break;
    case LAYOUTMODE_8:
        name = "LAYOUTMODE_8";
        break;
    case LAYOUTMODE_8_1:
        name = "LAYOUTMODE_8_1";
        break;
    case LAYOUTMODE_9:
        name = "LAYOUTMODE_9";
        break;
    case LAYOUTMODE_12:
        name = "LAYOUTMODE_12";
        break;
    case LAYOUTMODE_12_1:
        name = "LAYOUTMODE_12_1";
        break;
    case LAYOUTMODE_14:
        name = "LAYOUTMODE_14";
        break;
    case LAYOUTMODE_16:
        name = "LAYOUTMODE_16";
        break;
    case LAYOUTMODE_25:
        name = "LAYOUTMODE_25";
        break;
    case LAYOUTMODE_32:
        name = "LAYOUTMODE_32";
        break;
    case LAYOUTMODE_36:
        name = "LAYOUTMODE_36";
        break;
    case LAYOUTMODE_32_2:
        name = "LAYOUTMODE_32_2";
        break;
    case LAYOUTMODE_64:
        name = "LAYOUTMODE_64";
        break;
    }
    return name;
}

/**
 * @brief CustomLayoutData::layoutInfo
 * @param key
 * @return 需要判断返回值是否有效
 */
const CustomLayoutInfo &CustomLayoutData::layoutInfo(const CustomLayoutKey &key) const
{
    return m_allLayouts.find(key);
}

bool CustomLayoutData::isChannelEnable(const CustomLayoutKey &key, int channel) const
{
    const CustomLayoutInfo &info = m_allLayouts.find(key);
    if (info.isValid()) {
        return info.isChannelEnable(channel);
    }

    return false;
}

bool CustomLayoutData::isSingleLayout(const CustomLayoutKey &key) const
{
    const CustomLayoutInfo &info = m_allLayouts.find(key);
    if (info.isValid()) {
        return info.isSingleLayout();
    }

    return false;
}

CustomLayoutInfoList CustomLayoutData::allLayouts() const
{
    return m_allLayouts;
}

void CustomLayoutData::setAllLayouts(const CustomLayoutInfoList &layouts)
{
    m_allLayouts = layouts;
}

void CustomLayoutData::swapChannel(const CustomLayoutKey &key, int index1, int channel1, int index2, int channel2)
{
    CustomLayoutInfo &info = m_allLayouts.find(key);
    if (info.isValid()) {
        return info.swapChannel(index1, channel1, index2, channel2);
    } else {
        qMsWarning() << "invalid key:" << key;
    }
}

/**
 * @brief CustomLayoutData::saveOldLayouts
 * 主要是为了兼容升级降级
 */
void CustomLayoutData::saveOldLayouts()
{
    mosaic mosaicArray[MaxAllLayout];
    memset(mosaicArray, 0, sizeof(mosaic) * MaxAllLayout);
    int count = 0;
    for (int i = 0; i < m_allLayouts.size(); ++i) {
        const auto &info = m_allLayouts.at(i);
        if (info.screen() == SCREEN_MAIN && info.type() == CustomLayoutKey::DefaultType) {
            mosaic &m = mosaicArray[count];
            m.layoutmode = info.defaultLayoutMode();
            if (m.layoutmode == -1) {
                qMsWarning() << "error layoutmode:" << m.layoutmode;
                continue;
            }
            for (int j = 0; j < MAX_CAMERA; ++j) {
                m.channels[j] = info.channel(j);
            }
        }
        count++;
    }
    update_mosaics(SQLITE_FILE_NAME, mosaicArray, count);
}

void CustomLayoutData::saveAllLayouts()
{
    custom_mosaic mosaics[MaxAllLayout];
    memset(mosaics, 0, sizeof(mosaics));
    int count = m_allLayouts.size();
    for (int i = 0; i < m_allLayouts.size(); ++i) {
        const CustomLayoutInfo &info = m_allLayouts.at(i);
        //兼容数据库存储单引号错误
        QString amendName = info.name().replace("'", "''");
        custom_mosaic &m = mosaics[i];
        snprintf(m.name, sizeof(m.name), "%s", amendName.toStdString().c_str());
        m.screen = info.screen();
        m.type = info.type();
        m.baseRow = info.baseRow();
        m.baseColumn = info.baseColumn();
        const auto positions = info.positions();
        int j = 0;
        for (auto iter2 = positions.constBegin(); iter2 != positions.constEnd(); ++iter2) {
            const VideoPosition &p = iter2.key();
            mosaic_position &mp = m.positions[j];
            mp.index = p.index;
            mp.row = p.row;
            mp.column = p.column;
            mp.rowSpan = p.rowSpan;
            mp.columnSpan = p.columnSpan;
            j++;
        }
        for (int k = 0; k < MAX_CAMERA; ++k) {
            m.channels[k] = info.channel(k);
        }
    }
    qMsDebug() << QString("count: %1").arg(count);
    write_custom_mosaics_64Screen(SQLITE_FILE_NAME, mosaics, count);
}

void CustomLayoutData::clearStreamType()
{
    m_streamMap.clear();
}

void CustomLayoutData::saveStreamType(const StreamKey &key, int streamType)
{
    m_streamMap.insert(key, streamType);
}

int CustomLayoutData::streamType(const StreamKey &key) const
{
    if (m_streamMap.contains(key)) {
        return m_streamMap.value(key);
    } else {
        return -1;
    }
}

int CustomLayoutData::customStream(int channel)
{
    for (auto iter = m_streamMap.constBegin(); iter != m_streamMap.constEnd(); ++iter) {
        const StreamKey &key = iter.key();
        if (key.currentChannel() == channel) {
            return iter.value();
        }
    }
    return -1;
}

void CustomLayoutData::addLayoutMode_1()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_1)) {
        qMsWarning() << "No LAYOUTMODE_1";
        return;
    }
    int baseRow = 1;
    int baseColumn = 1;
    CustomLayoutInfo info("LAYOUTMODE_1", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    for (int r = 0; r < baseRow; ++r) {
        for (int c = 0; c < baseColumn; ++c) {
            info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
        }
    }
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_1);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_4()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_4)) {
        qMsWarning() << "No LAYOUTMODE_4";
        return;
    }
    int baseRow = 2;
    int baseColumn = 2;
    CustomLayoutInfo info("LAYOUTMODE_4", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    for (int r = 0; r < baseRow; ++r) {
        for (int c = 0; c < baseColumn; ++c) {
            info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
        }
    }
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_4);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_8()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_8)) {
        qMsWarning() << "No LAYOUTMODE_8";
        return;
    }
    int baseRow = 4;
    int baseColumn = 5;
    CustomLayoutInfo info("LAYOUTMODE_8", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    info.insertPosition(VideoPosition(0, 0, 2, 2), QRectF());
    info.insertPosition(VideoPosition(0, 2, 2, 2), QRectF());
    info.insertPosition(VideoPosition(0, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(1, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 0, 2, 2), QRectF());
    info.insertPosition(VideoPosition(2, 2, 2, 2), QRectF());
    info.insertPosition(VideoPosition(2, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 4, 1, 1), QRectF());
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_8);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_8_1()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_8_1)) {
        qMsWarning() << "No LAYOUTMODE_8_1";
        return;
    }
    int baseRow = 4;
    int baseColumn = 4;
    CustomLayoutInfo info("LAYOUTMODE_8_1", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    info.insertPosition(VideoPosition(0, 0, 3, 3), QRectF());
    info.insertPosition(VideoPosition(0, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(1, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 0, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 1, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 2, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 3, 1, 1), QRectF());
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_8_1);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_9()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_9)) {
        qMsWarning() << "No LAYOUTMODE_9";
        return;
    }
    int baseRow = 3;
    int baseColumn = 3;
    CustomLayoutInfo info("LAYOUTMODE_9", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    for (int r = 0; r < baseRow; ++r) {
        for (int c = 0; c < baseColumn; ++c) {
            info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
        }
    }
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_9);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_12()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_12)) {
        qMsWarning() << "No LAYOUTMODE_12";
        return;
    }
    int baseRow = 3;
    int baseColumn = 4;
    CustomLayoutInfo info("LAYOUTMODE_12", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    for (int r = 0; r < baseRow; ++r) {
        for (int c = 0; c < baseColumn; ++c) {
            info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
        }
    }
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_12);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_12_1()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_12_1)) {
        qMsWarning() << "No LAYOUTMODE_12_1";
        return;
    }
    int baseRow = 4;
    int baseColumn = 5;
    CustomLayoutInfo info("LAYOUTMODE_12_1", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    info.insertPosition(VideoPosition(0, 0, 3, 3), QRectF());
    info.insertPosition(VideoPosition(0, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(0, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(1, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(1, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 0, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 1, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 2, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 4, 1, 1), QRectF());
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_12_1);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_14()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_14)) {
        qMsWarning() << "No LAYOUTMODE_14";
        return;
    }
    int baseRow = 4;
    int baseColumn = 5;
    CustomLayoutInfo info("LAYOUTMODE_14", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    info.insertPosition(VideoPosition(0, 0, 2, 2), QRectF());
    info.insertPosition(VideoPosition(0, 2, 2, 2), QRectF());
    info.insertPosition(VideoPosition(0, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(1, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 0, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 1, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 2, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 0, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 1, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 2, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 4, 1, 1), QRectF());
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_14);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_16()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_16)) {
        qMsWarning() << "No LAYOUTMODE_16";
        return;
    }
    int baseRow = 4;
    int baseColumn = 4;
    CustomLayoutInfo info("LAYOUTMODE_16", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    for (int r = 0; r < baseRow; ++r) {
        for (int c = 0; c < baseColumn; ++c) {
            info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
        }
    }
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_16);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_25()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_25)) {
        qMsWarning() << "No LAYOUTMODE_25";
        return;
    }
    int baseRow = 5;
    int baseColumn = 5;
    CustomLayoutInfo info("LAYOUTMODE_25", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    for (int r = 0; r < baseRow; ++r) {
        for (int c = 0; c < baseColumn; ++c) {
            info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
        }
    }
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_25);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_32()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_32)) {
        qMsWarning() << "No LAYOUTMODE_32";
        return;
    }
    int baseRow = 4;
    int baseColumn = 8;
    CustomLayoutInfo info("LAYOUTMODE_32", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    for (int r = 0; r < baseRow; ++r) {
        for (int c = 0; c < baseColumn; ++c) {
            info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
        }
    }
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_32);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_32_2()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_32_2)) {
        qMsWarning() << "No LAYOUTMODE_32_2";
        return;
    }
    int baseRow = 7;
    int baseColumn = 7;
    CustomLayoutInfo info("LAYOUTMODE_32_2", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    info.insertPosition(VideoPosition(0, 0, 3, 3), QRectF());
    info.insertPosition(VideoPosition(0, 3, 2, 2), QRectF());
    info.insertPosition(VideoPosition(0, 5, 2, 2), QRectF());
    info.insertPosition(VideoPosition(2, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 5, 1, 1), QRectF());
    info.insertPosition(VideoPosition(2, 6, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 0, 2, 2), QRectF());
    info.insertPosition(VideoPosition(3, 2, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 5, 1, 1), QRectF());
    info.insertPosition(VideoPosition(3, 6, 1, 1), QRectF());
    info.insertPosition(VideoPosition(4, 2, 1, 1), QRectF());
    info.insertPosition(VideoPosition(4, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(4, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(4, 5, 1, 1), QRectF());
    info.insertPosition(VideoPosition(4, 6, 1, 1), QRectF());
    info.insertPosition(VideoPosition(5, 0, 1, 1), QRectF());
    info.insertPosition(VideoPosition(5, 1, 1, 1), QRectF());
    info.insertPosition(VideoPosition(5, 2, 1, 1), QRectF());
    info.insertPosition(VideoPosition(5, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(5, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(5, 5, 1, 1), QRectF());
    info.insertPosition(VideoPosition(5, 6, 1, 1), QRectF());
    info.insertPosition(VideoPosition(6, 0, 1, 1), QRectF());
    info.insertPosition(VideoPosition(6, 1, 1, 1), QRectF());
    info.insertPosition(VideoPosition(6, 2, 1, 1), QRectF());
    info.insertPosition(VideoPosition(6, 3, 1, 1), QRectF());
    info.insertPosition(VideoPosition(6, 4, 1, 1), QRectF());
    info.insertPosition(VideoPosition(6, 5, 1, 1), QRectF());
    info.insertPosition(VideoPosition(6, 6, 1, 1), QRectF());
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_32_2);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}

void CustomLayoutData::addLayoutMode_64()
{
    if (!m_mosaicMap.contains(LAYOUTMODE_64)) {
        qMsWarning() << "No LAYOUTMODE_64";
        return;
    }
    int baseRow = 8;
    int baseColumn = 8;
    CustomLayoutInfo info("LAYOUTMODE_64", SCREEN_MAIN, baseRow, baseColumn);
    info.setType(CustomLayoutKey::DefaultType);
    for (int r = 0; r < baseRow; ++r) {
        for (int c = 0; c < baseColumn; ++c) {
            info.insertPosition(VideoPosition(r, c, 1, 1), QRectF());
        }
    }
    for (int i = 0; i < MAX_CAMERA; ++i) {
        const mosaic &m = m_mosaicMap.value(LAYOUTMODE_64);
        info.insertChannel(i, m.channels[i]);
    }
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
    //
    info.setScreen(SCREEN_SUB);
    if (!layoutInfo(info.key()).isValid()) {
        m_allLayouts.append(info);
    }
}
