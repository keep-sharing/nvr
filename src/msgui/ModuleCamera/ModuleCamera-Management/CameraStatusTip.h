#ifndef CAMERASTATUSTIP_H
#define CAMERASTATUSTIP_H

#include <QWidget>

namespace Ui {
class CameraStatusTip;
}

class CameraStatusTip : public QWidget {
    Q_OBJECT

public:
    explicit CameraStatusTip(QWidget *parent = nullptr);
    ~CameraStatusTip();

    static CameraStatusTip *instance();

    void setState(int state, const QString &text);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    static CameraStatusTip *self;

    Ui::CameraStatusTip *ui;

    int m_width = 0;
    int m_height = 0;
};

#endif // CAMERASTATUSTIP_H
