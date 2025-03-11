#ifndef IMAGECONFIGURATION_H
#define IMAGECONFIGURATION_H

#include "BaseDialog.h"
#include <QTimer>

class CentralMessage;

namespace Ui {
class ImageConfiguration;
}

class ImageConfiguration : public BaseDialog {
    Q_OBJECT

public:
    explicit ImageConfiguration(QWidget *parent = 0);
    ~ImageConfiguration();

    void showImageInfo(int channel, const QRect &videoGeometry);

    void processMessage(MessageReceive *message);

protected:
    void ON_RESPONSE_FLAG_GET_IMAGEPARAM(MessageReceive *message);

    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void escapePressed() override;

    bool isMoveToCenter() override;
    bool isAddToVisibleList() override;

private:
    int toPercentValue(int value);
    int fromPercentValue(int value);

private slots:
    void onLanguageChanged();

    void onSetImageInfo();
    void onSendTimer();

    void on_pushButton_default_clicked();
    void on_pushButton_close_clicked();

private:
    Ui::ImageConfiguration *ui;

    int m_currentChannel = -1;
    bool m_sendChange = true;

    QTimer *m_sendTimer;

    bool m_titlePressed = false;
    QPoint m_titlePressedDistance;
};

#endif // IMAGECONFIGURATION_H
