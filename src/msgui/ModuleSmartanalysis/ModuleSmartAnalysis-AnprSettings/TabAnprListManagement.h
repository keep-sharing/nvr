#ifndef TABANPRLISTMANAGEMENT_H
#define TABANPRLISTMANAGEMENT_H

#include "anprbasepage.h"
#include "anprthread.h"
#include "progressdialog.h"

namespace Ui {
class AnprListManagementPage;
}

class TabAnprListManagement : public AnprBasePage {
    Q_OBJECT

    enum LicenseColumn {
        ColumnCheck,
        ColumnLicense,
        ColumnType,
        ColumnEdit,
        ColumnDelete
    };

public:
    explicit TabAnprListManagement(QWidget *parent = nullptr);
    ~TabAnprListManagement();

    virtual void initializeData(int channel) override;

protected:
    void showEvent(QShowEvent *) override;

private:
    //清空搜索条件，重新读数据库，刷新表格
    void resetAndReadLicense();
    //不清空搜索条件，重新读数据库，刷新表格
    void readLicense();
    //只刷新表格
    void updateLicenseTable();

private slots:
    void onLanguageChanged();

    void onTableItemClicked(int row, int column);

    void onImportError(const QString &error);
    void onImportDealRepeat();
    void onImportProgress(int progress);
    void onImportResult(int succeedCount, int failedCount, int errorCount, bool isReachLimit, bool isCancel);

    void onDeleteProgress(int progress);
    void onDeleteResult();

    void onCancelClicked();

    void on_toolButton_firstPage_clicked();
    void on_toolButton_previousPage_clicked();
    void on_toolButton_nextPage_clicked();
    void on_toolButton_lastPage_clicked();
    void on_pushButton_go_clicked();

    void on_pushButton_search_clicked();
    void on_pushButton_add_clicked();
    void on_pushButton_deleteList_clicked();
    void on_pushButton_import_clicked();
    void on_pushButton_export_clicked();
    void on_pushButton_back_clicked();

    void on_pushButton_template_clicked();

private:
    Ui::AnprListManagementPage *ui;

    QMap<QString, anpr_list> m_allLicenseMap;
    QList<anpr_list> m_searchedLicenseList;

    int m_pageCount = 0;
    int m_pageIndex = 0;

    ProgressDialog *m_progressDialog = nullptr;
    AnprThread *m_anprThread = nullptr;
    QStringList m_tempErrorList;
};

#endif // TABANPRLISTMANAGEMENT_H
