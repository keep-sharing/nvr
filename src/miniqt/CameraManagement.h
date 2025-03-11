#ifndef CAMERAMANAGEMENT_H
#define CAMERAMANAGEMENT_H

#include <QDialog>
#include <QMap>
#include "CameraModel.h"
#include <QSettings>

namespace Ui {
class CameraManagement;
}

class CameraManagement : public QDialog
{
    Q_OBJECT

public:
    explicit CameraManagement(QWidget *parent = nullptr);
    ~CameraManagement();

protected:
    void showEvent(QShowEvent *) override;

private:
    void saveCameraList();

private slots:
    void readCameraList();

    void on_pushButtonAdd_clicked();
    void onTableViewClicked(const QModelIndex &index);

private:
    Ui::CameraManagement *ui;

    CameraModel *m_cameraModel = nullptr;
    QMap<int, int> m_channelMap;

    QSettings *m_settings = nullptr;
};

#endif // CAMERAMANAGEMENT_H
