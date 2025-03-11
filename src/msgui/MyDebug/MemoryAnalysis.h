#ifndef MEMORYANALYSIS_H
#define MEMORYANALYSIS_H

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QTextStream>

#define gMemoryAnalysis MemoryAnalysis::instance()

class MemoryHead {
public:
    MemoryHead();
    MemoryHead(const QString &file, int line);

    QString file() const;
    int line() const;

    bool operator<(const MemoryHead &other) const;

private:
    QString m_file;
    int m_line = 0;
};
class MemoryInfo {
public:
    MemoryInfo();
    MemoryInfo(const QString &file, int line, int size);

    MemoryHead head() const;

    int size() const;

private:
    QString m_file;
    int m_line = 0;
    int m_size = 0;
};

class MemoryAnalysis : public QObject {
    Q_OBJECT
public:
    explicit MemoryAnalysis(QObject *parent = nullptr);

    static MemoryAnalysis &instance();

    void addMemoryInfo(void *address, int size, const char *file, int line);
    void removeMemoryInfo(void *address, const char *file, int line);
    void replaceMemoryInfo(void *oldAddress, void *newAddress, int size, const char *file, int line);

    void makeReport();

    qint64 allMallocSize() const;

signals:

private slots:
    void onTimer();

private:
    QMap<void *, MemoryInfo> m_mapMemoryInfo;
    QMap<MemoryHead, QMap<QDateTime, qint64>> m_mapRecord;
    qint64 m_allMallocSize = 0;

    QTimer *m_timer = nullptr;
};

#endif // MEMORYANALYSIS_H
