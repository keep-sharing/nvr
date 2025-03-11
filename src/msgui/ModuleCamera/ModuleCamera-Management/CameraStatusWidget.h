#ifndef CAMERASTATUSWIDGET_H
#define CAMERASTATUSWIDGET_H

#include <QWidget>
#include "authentication.h"

extern "C" {
#include "msg.h"
}

class CameraStatusTip;

namespace Ui {
class CameraStatusWidget;
}

class CameraStatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraStatusWidget(QWidget *parent = nullptr);
    ~CameraStatusWidget();

    void setState(int state);
    int stateValue();

    void setStateString(const IPC_CONNECT_STATUS &status);

protected:
    bool eventFilter(QObject *, QEvent *) override;

private:
    Ui::CameraStatusWidget *ui;

    int m_state = 0;
    QString m_disconnectedStateText;
    CameraStatusTip *m_statusTip = nullptr;
};

#endif // CAMERASTATUSWIDGET_H
