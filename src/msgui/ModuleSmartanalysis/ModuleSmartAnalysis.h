#ifndef MODULESMARTANALYSIS_H
#define MODULESMARTANALYSIS_H

#include "basesetting.h"

#include <QMap>

class AbstractSettingPage;

namespace Ui {
class SmartAnalysisSetting;
}

class ModuleSmartAnalysis : public BaseSetting {
    Q_OBJECT

public:
    enum ItemType {
        ItemNone,

        ItemAnalysisSettings,
        ItemAnalysisSettings_Anpr,
        itemAnalysisSettings_FaceDetection,
        ItemAnalysisSettings_PeopleCounting,
        ItemAnalysisSettings_Heatmap,
        ItemAnalysisSettings_Pos,

        ItemAnalysisSearch,
        ItemAnalysisSearch_Anpr,
        itemAnalysisSearch_FaceDetection,
        ItemAnalysisSearch_PeopleCounting,
        ItemAnalysisSearch_Heatmap,
        ItemAnalysisSearch_Pos
    };

    explicit ModuleSmartAnalysis(QWidget *parent = 0);
    ~ModuleSmartAnalysis();

    NetworkResult dealNetworkCommond(const QString &commond) override;

    QList<SettingItemInfo> itemList() override;
    void setCurrentItem(int item_id) override;
    int currentItem() override;
    bool canAutoLogout() override;
    void closeSetting() override;
    void closeCurrentPage() override;

protected:
    bool isAddToVisibleList() override;

private:
    Ui::SmartAnalysisSetting *ui;

    ItemType m_currentItemType = ItemNone;
    QMap<ItemType, AbstractSettingPage *> m_pageMap;
};

#endif // MODULESMARTANALYSIS_H
