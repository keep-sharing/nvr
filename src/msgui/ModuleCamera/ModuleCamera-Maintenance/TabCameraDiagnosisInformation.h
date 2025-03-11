#ifndef TABCAMERADIAGNOSISINFORMATION_H
#define TABCAMERADIAGNOSISINFORMATION_H

#include "AbstractSettingTab.h"
#include <QEventLoop>
#include "MaintenanceTableView.h"
extern "C" {
#include "msg.h"
}

namespace Ui {
class TabCameraDiagnosisInformation;
}

class TabCameraDiagnosisInformation : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabCameraDiagnosisInformation(QWidget *parent = nullptr);
    ~TabCameraDiagnosisInformation();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

    bool isCloseable() override;
    bool isChangeable() override;
    bool canAutoLogout() override;

protected:
    void onLanguageChanged() override;
private:
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_DIAGNOSE(MessageReceive *message);
    void setUpgradeEnabled(bool enable);
private slots:
    void on_pushButtonBrowseExport_clicked();
    void on_pushButtonExport_clicked();

    void on_pushButtonRefresh_clicked();
    void on_pushButtonBack_clicked();


private:
    Ui::TabCameraDiagnosisInformation *ui;
    QList<resq_get_ipcdev> m_allCameraList;

    QString m_filePath;
    int m_currentChannel = -1;
    QMap<int, int> m_channelMap;
    QMap<int, MaintenanceTablelItemInfo> m_stateMap;

    QEventLoop m_eventLoop;

    bool m_isCancel = false;
    bool m_isAboutToClose = false;
};

#endif // TABCAMERADIAGNOSISINFORMATION_H
