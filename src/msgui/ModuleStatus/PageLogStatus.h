#ifndef PAGELOGSTATUS_H
#define PAGELOGSTATUS_H

#include "AbstractSettingPage.h"

extern "C" {
#include "msg.h"
}

class ProgressDialog;
class LogDetail;

namespace Ui {
class LogStatus;
}

class PageLogStatus : public AbstractSettingPage {
    Q_OBJECT

    enum LogColumn {
        ColumnCheck,
        ColumnNumber,
        ColumnMainType,
        ColumnTime,
        ColumnSubType,
        ColumnParameter,
        ColumnChannel,
        ColumnUser,
        ColumnRemoteHostIP,
        ColumnDetails
    };

public:
    explicit PageLogStatus(QWidget *parent = 0);
    ~PageLogStatus();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

    static QString mainTypeString(int type);
    static QString subTypeString(int type);

private:
    void ON_RESPONSE_FLAG_LOG_SEARCH(MessageReceive *message);
    void ON_RESPONSE_FLAG_LOG_EXPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_LOG_GET_DETAIL(MessageReceive *message);

    void ON_RESPONSE_FLAG_PROGRESS_LOG(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_LOG_EXPORT(MessageReceive *message);

    void searchLogPage(int page);
    void updateTable();

private slots:
    void onLanguageChanged() override;

    void onTableItemClicked(int row, int column);
    void onTableItemDoubleClicked(int row, int column);

    //search
    void onSearchCanceled();

    //export
    void onExportCanceled();

    //log detail
    void onPreviousLog();
    void onNextLog();

    void on_comboBox_maintype_activated(int index);

    void on_toolButton_firstPage_clicked();
    void on_toolButton_previousPage_clicked();
    void on_toolButton_nextPage_clicked();
    void on_toolButton_lastPage_clicked();
    void on_pushButton_go_clicked();

    void on_pushButton_export_clicked();
    void on_pushButton_search_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::LogStatus *ui;

    search_criteria m_criteria;
    QList<log_data> m_listLog;

    int m_allCount = 0;
    int m_pageIndex = 0;
    int m_pageCount = 0;

    ProgressDialog *m_progressSearch = nullptr;
    ProgressDialog *m_progressExport = nullptr;
    LogDetail *m_logDetail = nullptr;
};

#endif // PAGELOGSTATUS_H
