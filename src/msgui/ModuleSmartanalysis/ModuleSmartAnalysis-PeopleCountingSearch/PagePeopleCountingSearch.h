#ifndef PAGEPEOPLECOUNTINGSEARCH_H
#define PAGEPEOPLECOUNTINGSEARCH_H

#include "AbstractSettingPage.h"
#include "PeopleCountingData.h"

class PeopleCountingCameraResult;
class PeopleCountingRegionResult;
class PeopleCountingGroupResult;

namespace Ui {
class PeopleCountingSearch;
}

class PagePeopleCountingSearch : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PagePeopleCountingSearch(QWidget *parent = 0);
    ~PagePeopleCountingSearch();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SET_PEOPLECNT_CACHETODB(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_PEOPLE_REPORT(MessageReceive *message);

    bool isGroupIncludeMoreChannels();

    void showSearchResult();

private slots:
    void onLanguageChanged() override;

    void on_comboBoxSearchType_indexSet(int index);
    void on_comboBoxLengthOfStayRegion_indexSet(int index);

    //search
    void on_pushButtonSearch_clicked();
    void on_pushButtonBack_clicked();

    //backup
    void on_pushButtonBackupAll_clicked();
    void on_pushButtonBackup_clicked();

    //back
    void on_pushButtonResultBack_clicked();

private:
    Ui::PeopleCountingSearch *ui;

    QEventLoop m_eventLoop;

    PeopleCountingCameraResult *m_cameraResult = nullptr;
    PeopleCountingRegionResult *m_regionResult = nullptr;
    PeopleCountingGroupResult *m_groupResult = nullptr;

    PEOPLECNT_SEARCH_TYPE_E m_searchType;
    ReportType m_recordType;
    StatisticsType m_statisticsType;

    QList<int> m_checkedList;
    QList<int> m_checkedCameraList;
    QMap<int ,QString> m_textMap;
    int m_currentChannel = -1;

};

#endif // PAGEPEOPLECOUNTINGSEARCH_H
