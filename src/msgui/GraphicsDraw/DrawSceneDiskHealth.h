#ifndef DRAWSCENEDISKHEALTH_H
#define DRAWSCENEDISKHEALTH_H

#include <QDateTime>
#include <QGraphicsScene>

class DrawItemDiskHealth;
struct disk_temperature;

class DrawSceneDiskHealth : public QGraphicsScene {
    Q_OBJECT

public:
    explicit DrawSceneDiskHealth(QObject *parent = nullptr);

    void showDiskHealthMap(struct disk_temperature *temperatureList);

private:
    DrawItemDiskHealth *m_diskItem = nullptr;
};

#endif // DRAWSCENEDISKHEALTH_H
