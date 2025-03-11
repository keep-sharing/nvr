#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "BottomBar.h"
#include "CameraInfo.h"
#include "CameraManagement.h"
#include <QMainWindow>
#include <QMap>

extern "C"
{
#include "recortsp.h"
#include "vapi.h"
}

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

int callback_start_stream(int chan_id);
int callback_stop_stream(int chan_id);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static int vapiInitialize(SCREEN_RES_E res);

protected:
    void resizeEvent(QResizeEvent *) override;
    void paintEvent(QPaintEvent *) override;

private slots:
    void onLayoutChanged(int row, int column);
    void onCameraManagement();

private:
    Ui::MainWindow *ui;

    BottomBar        *m_bottomBar        = nullptr;
    CameraManagement *m_cameraManagement = nullptr;
};
#endif // MAINWINDOW_H
