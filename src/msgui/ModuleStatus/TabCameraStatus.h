#ifndef TABCAMERASTATUS_H
#define TABCAMERASTATUS_H

#include "AbstractSettingTab.h"
#include <QTimer>
#include <QWidget>

namespace Ui {
class PageCameraStatus;
}

class TabCameraStatus : public AbstractSettingTab {
    Q_OBJECT

public:
    enum CameraColumn {
        ColumnChannel,
        ColumnName,
        ColumnIP,
        ColumnRecord,
        ColumnFrameRate,
        ColumnBitRate,
        ColumnFrameSize,
        ColumnStatus
    };

    explicit TabCameraStatus(QWidget *parent = 0);
    ~TabCameraStatus();

    void initializeData();

    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);

    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onTimerout();

private:
    Ui::PageCameraStatus *ui;

    QTimer *m_timer = nullptr;
};

#endif // TABCAMERASTATUS_H
