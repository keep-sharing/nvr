#ifndef CAMERAINFOMANAGEMENT_H
#define CAMERAINFOMANAGEMENT_H

#include <QObject>

class CameraInfoManagement : public QObject {
    Q_OBJECT

public:
    explicit CameraInfoManagement(QObject *parent = nullptr);

    static CameraInfoManagement &instance();

    void updateData(int row);

signals:
    void dataChanged(int row);
};

#endif // CAMERAINFOMANAGEMENT_H
