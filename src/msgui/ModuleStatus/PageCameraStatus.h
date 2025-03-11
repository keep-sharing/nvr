#ifndef PAGECAMERASTATUS_H
#define PAGECAMERASTATUS_H

#include "AbstractSettingPage.h"

namespace Ui {
class CameraStatus;
}

class PageCameraStatus : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PageCameraStatus(QWidget *parent = 0);
    ~PageCameraStatus();

    void initializeData() override;

    bool canAutoLogout() override;

    void dealMessage(MessageReceive *message) override;

private slots:
    void onLanguageChanged();

    void on_pushButton_back_clicked();

    void onTabClicked(int index);

private:
    Ui::CameraStatus *ui;
};

#endif // PAGECAMERASTATUS_H
