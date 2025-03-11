#include "MemoryAnalysis.h"
#include <QMutex>
#include <QFile>
#include "MyDebug.h"

QMutex gMemoryMutex;

MemoryAnalysis::MemoryAnalysis(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    m_timer->start(1000 * 60 * 5);
}

MemoryAnalysis &MemoryAnalysis::instance()
{
    static MemoryAnalysis self;
    return self;
}

void MemoryAnalysis::addMemoryInfo(void *address, int size, const char *file, int line)
{
    QMutexLocker locker(&gMemoryMutex);
    MemoryInfo info(file, line, size);
    m_allMallocSize += info.size();
    m_mapMemoryInfo.insert(address, info);
}

void MemoryAnalysis::removeMemoryInfo(void *address, const char *file, int line)
{
    Q_UNUSED(file)
    Q_UNUSED(line)

    QMutexLocker locker(&gMemoryMutex);
    const auto &info = m_mapMemoryInfo.value(address);
    m_allMallocSize -= info.size();
    m_mapMemoryInfo.remove(address);
}

void MemoryAnalysis::replaceMemoryInfo(void *oldAddress, void *newAddress, int size, const char *file, int line)
{
    QMutexLocker locker(&gMemoryMutex);
    const auto &oldInfo = m_mapMemoryInfo.value(oldAddress);
    m_allMallocSize -= oldInfo.size();
    m_mapMemoryInfo.remove(oldAddress);
    MemoryInfo info(file, line, size);
    m_allMallocSize += info.size();
    m_mapMemoryInfo.insert(newAddress, info);
}

void MemoryAnalysis::makeReport()
{
    QFile file("/mnt/nand/mem_report");
    if (!file.open(QFile::WriteOnly)) {
        qMsWarning() << file.errorString();
        return;
    }
    QTextStream stream(&file);
    stream << "current time:" << QDateTime::currentDateTime().toString("yyyy-MM-dd_HH:mm:ss") << "\r\n";
    for (auto iter = m_mapRecord.constBegin(); iter != m_mapRecord.constEnd(); ++iter) {
        const auto &key = iter.key();
        const auto &value = iter.value();
        stream << "----" << key.file() << " " << key.line() << "----\r\n";
        for (auto iter2 = value.constBegin(); iter2 != value.constEnd(); ++iter2) {
            const QDateTime &key2 = iter2.key();
            const qint64 &value2 = iter2.value();
            stream << key2.toString("yyyy-MM-dd_HH:mm:ss") << " " << value2 << "\r\n";
        }
    }
    file.close();
}

qint64 MemoryAnalysis::allMallocSize() const
{
    return m_allMallocSize;
}

void MemoryAnalysis::onTimer()
{
    qMsDebug() << "make memory report begin";
    QMap<MemoryHead, qint64> mapSize;
    QMutexLocker locker(&gMemoryMutex);
    for (auto iter = m_mapMemoryInfo.constBegin(); iter != m_mapMemoryInfo.constEnd(); ++iter) {
        const auto &value = iter.value();
        qint64 & size = mapSize[value.head()];
        size += value.size();
    }

    for (auto iter = mapSize.constBegin(); iter != mapSize.constEnd(); ++iter) {
        const auto &key = iter.key();
        const auto &value = iter.value();
        auto &map = m_mapRecord[key];
        if (map.isEmpty()) {
            map.insert(QDateTime::currentDateTime(), value);
        } else {
            auto iter2 = map.end();
            iter2--;
            qint64 size = iter2.value();
            if (size != value) {
                map.insert(QDateTime::currentDateTime(), value);
            }
        }
    }

    makeReport();
    qMsDebug() << "make memory report end";
}

MemoryHead::MemoryHead()
{

}

MemoryHead::MemoryHead(const QString &file, int line)
{
    m_file = file;
    m_line = line;
}

QString MemoryHead::file() const
{
    return m_file;
}

int MemoryHead::line() const
{
    return m_line;
}

bool MemoryHead::operator <(const MemoryHead &other) const
{
    if (m_file != other.file()) {
        return m_file < other.file();
    } else {
        return m_line < other.line();
    }
}

MemoryInfo::MemoryInfo()
{

}

MemoryInfo::MemoryInfo(const QString &file, int line, int size)
{
    m_file = QString(file);
    m_line = line;
    m_size = size;
}

MemoryHead MemoryInfo::head() const
{
    return MemoryHead(m_file, m_line);
}

int MemoryInfo::size() const
{
    return m_size;
}
