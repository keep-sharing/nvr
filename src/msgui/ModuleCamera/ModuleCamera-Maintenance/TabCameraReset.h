#ifndef TABCAMERARESET_H
#define TABCAMERARESET_H

#include "AbstractSettingTab.h"
#include <QEventLoop>
#include "MaintenanceTableView.h"
extern "C" {
#include "msg.h"
}

namespace Ui {
class TabCameraReset;
}

class TabCameraReset : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabCameraReset(QWidget *parent = nullptr);
    ~TabCameraReset();
    void initializeData() override;

    void processMessage(MessageReceive *message) override;

    bool isCloseable() override;
    bool isChangeable() override;
    bool canAutoLogout() override;

protected:
    void onLanguageChanged() override;
private:
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_RESET(MessageReceive *message);
    void setUpgradeEnabled(bool enable);

private slots:
    void on_pushButtonReset_clicked();
    void on_pushButtonRefresh_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::TabCameraReset *ui;
    QList<resq_get_ipcdev> m_allCameraList;

    QString m_filePath;
    int m_currentChannel = -1;
    QMap<int, int> m_channelMap;
    QMap<int, MaintenanceTablelItemInfo> m_stateMap;

    QEventLoop m_eventLoop;

    bool m_isCancel = false;
    bool m_isAboutToClose = false;
};

#endif // TABCAMERARESET_H
