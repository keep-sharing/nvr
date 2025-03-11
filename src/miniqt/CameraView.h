#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include <QTableView>

class CameraView : public QTableView {
    Q_OBJECT

public:
    explicit CameraView(QWidget *parent = nullptr);
};

#endif // CAMERAVIEW_H
