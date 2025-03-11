#ifndef TABCAMERAEXPORTCONFIGURATION_H
#define TABCAMERAEXPORTCONFIGURATION_H

#include "AbstractSettingTab.h"
#include <QEventLoop>
#include "MaintenanceTableView.h"
extern "C" {
#include "msg.h"
}

namespace Ui {
class TabCameraExportConfiguration;
}

class TabCameraExportConfiguration : public AbstractSettingTab {
    Q_OBJECT
    enum ConfigurationMode {
        IMPORT_CONFIGURATION,
        EXPORT_CONFIGURATION
    };

public:
    explicit TabCameraExportConfiguration(QWidget *parent = nullptr);
    ~TabCameraExportConfiguration() override;

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

    bool isCloseable() override;
    bool isChangeable() override;
    bool canAutoLogout() override;

protected:
    void onLanguageChanged() override;

private:
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_CFG(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_CFG(MessageReceive *message);
    void setUpgradeEnabled(bool enable);

    //0:import 1:export
    void importOrExportConfiguration(int mode, QString pwd);

private slots:
    void on_pushButtonBrowseImport_clicked();
    void on_pushButtonImport_clicked();
    void on_pushButtonBrowseExport_clicked();
    void on_pushButtonExport_clicked();

    void on_pushButtonRefresh_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::TabCameraExportConfiguration *ui;
    QList<resq_get_ipcdev> m_allCameraList;

    QString m_filePath;
    int m_currentChannel = -1;
    QMap<int, int> m_channelMap;
    QMap<int, MaintenanceTablelItemInfo> m_stateMap;

    QEventLoop m_eventLoop;

    bool m_isCancel = false;
    bool m_isAboutToClose = false;
};

#endif // TABCAMERAEXPORTCONFIGURATION_H
