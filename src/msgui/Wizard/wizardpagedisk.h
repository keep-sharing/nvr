#ifndef WIZARDPAGEDISK_H
#define WIZARDPAGEDISK_H

#include "abstractwizardpage.h"

namespace Ui {
class WizardPageDisk;
}

class WizardPageDisk : public AbstractWizardPage {
    Q_OBJECT

    enum DiskColumn {
        ColumnCheck,
        ColumnPort,
        ColumnVendor,
        ColumnStatus,
        ColumnTotal,
        ColumnFree,
        ColumnType
    };

public:
    explicit WizardPageDisk(QWidget *parent = nullptr);
    ~WizardPageDisk();

    void initializeData() override;
    void saveSetting() override;

    virtual void previousPage() override;
    virtual void nextPage() override;
    virtual void skipWizard() override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_DISK_INIT(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(MessageReceive *message);

    void refreshTable();

private slots:
    void onLanguageChanged();

    void onTableClicked(int row, int column);

    void on_pushButton_initialize_clicked();

private:
    Ui::WizardPageDisk *ui;
};

#endif // WIZARDPAGEDISK_H
