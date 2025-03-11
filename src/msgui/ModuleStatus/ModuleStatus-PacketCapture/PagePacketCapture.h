#ifndef PAGEPACKETCAPTURE_H
#define PAGEPACKETCAPTURE_H

#include "AbstractSettingPage.h"
#include <QEventLoop>

namespace Ui {
class PacketCapture;
}

class PagePacketCapture : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PagePacketCapture(QWidget *parent = nullptr);
    ~PagePacketCapture();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private slots:
    void onLanguageChanged() override;

    void onStarted();
    void onFinished();

    void on_pushButtonBrowse_clicked();
    void on_pushButtonStart_clicked();
    void on_pushButtonEnd_clicked();
    void on_pushButtonBack_clicked();

private:
    void updateFormData();
    void updateFormEnable();

private:
    Ui::PacketCapture *ui;

    QEventLoop m_eventLoop;
    int m_diskPort = 0;
};

#endif // PAGEPACKETCAPTURE_H
