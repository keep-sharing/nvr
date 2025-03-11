#ifndef PAGEHEATMAPSEARCH_H
#define PAGEHEATMAPSEARCH_H

#include "AbstractSettingPage.h"
#include <QEventLoop>
#include <heatmapthread.h>
#include "DrawSceneHeatMap.h"

extern "C" {
#include "msg.h"
}

class DrawView;
class DrawSceneHeatMap;

namespace Ui {
class HeatMapAnalysis;
}

class PageHeatMapSearch : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PageHeatMapSearch(QWidget *parent = nullptr);
    ~PageHeatMapSearch();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_SNAPHOST(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(MessageReceive *message);

    void clearInfo();
    bool isSupport(int channel);
    bool isConnected(int channel);

private slots:
    void onLanguageChanged() override;

    void onImageFinished(int max, int min, QImage colorImage, QImage heatmapImage);
    void onSaveSpaceHeatMapFinished(bool result);

    void on_comboBoxChannel_activated(int index);

    void on_pushButton_search_clicked();
    void on_pushButton_back_clicked();

    //backup
    void on_pushButtonBackupAll_clicked();
    void on_pushButtonBackup_clicked();

    //back
    void on_pushButtonResultBack_clicked();

private:
    Ui::HeatMapAnalysis *ui;

    QEventLoop m_eventLoop;

    DrawView *m_drawView = nullptr;
    DrawSceneHeatMap *m_drawScene = nullptr;
    QMap<int, DrawHeatMapData> m_drawHeatMapDataMap;
    QList<int> m_selectList;
    ms_heat_map_report m_report;

    HeatMapThread *m_heatMapThread = nullptr;

    int m_currentChannel = 0;
    int m_indexChannel = 0;
    bool m_isBackuping = false;
    int m_count = 0;
    int m_decoding = 0;
};

#endif // PAGEHEATMAPSEARCH_H
