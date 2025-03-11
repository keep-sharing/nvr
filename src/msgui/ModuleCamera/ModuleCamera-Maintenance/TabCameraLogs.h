#ifndef TabCameraLogs_H
#define TabCameraLogs_H
#include "AbstractSettingTab.h"
#include <QEventLoop>
#include "MaintenanceTableView.h"
extern "C" {
#include "msg.h"
}

namespace Ui {
class TabCameraLogs;
}

class TabCameraLogs : public AbstractSettingTab
{
    Q_OBJECT

public:
    explicit TabCameraLogs(QWidget *parent = nullptr);
    ~TabCameraLogs();
    void initializeData() override;

    void processMessage(MessageReceive *message) override;
    bool isCloseable() override;
    bool isChangeable() override;
    bool canAutoLogout() override;
protected:
    void onLanguageChanged() override;

private:
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_LOG(MessageReceive *message);
    void setUpgradeEnabled(bool enable);

private slots:
    void on_pushButtonBrowse_clicked();
    void on_pushButtonBackup_clicked();
    void on_comboBoxMainType_indexSet(int index);

    void on_pushButtonBack_clicked();
    void on_pushButtonRefresh_clicked();

private:
    Ui::TabCameraLogs *ui;
    QList<resq_get_ipcdev> m_allCameraList;

    QString m_filePath;
    int m_currentChannel = -1;
    QMap<int, int> m_channelMap;
    QMap<int, MaintenanceTablelItemInfo> m_stateMap;

    QEventLoop m_eventLoop;

    bool m_isCancel = false;
    bool m_isAboutToClose = false;
};

#endif // TabCameraLogs_H
