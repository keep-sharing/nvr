#include "TabAnprListManagement.h"
#include "ui_TabAnprListManagement.h"
#include "centralmessage.h"
#include "licenseplateadd.h"
#include "licenseplateedit.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyFileSystemDialog.h"
#include <QDateTime>
#include <QElapsedTimer>
#include <QTextStream>
#include <QtDebug>
#include <qmath.h>

TabAnprListManagement::TabAnprListManagement(QWidget *parent)
    : AnprBasePage(parent)
    , ui(new Ui::AnprListManagementPage)
{
    ui->setupUi(this);

    ui->comboBox_licenseType->clear();
    ui->comboBox_licenseType->addItem(GET_TEXT("ANPR/103038", "All"), "All");
    ui->comboBox_licenseType->addItem(GET_TEXT("ANPR/103039", "Black"), PARAM_MS_ANPR_TYPE_BLACK);
    ui->comboBox_licenseType->addItem(GET_TEXT("ANPR/103040", "White"), PARAM_MS_ANPR_TYPE_WHITE);

    //initialize table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("ANPR/103035", "License Plate");
    headerList << GET_TEXT("ANPR/103041", "Plate Type");
    headerList << GET_TEXT("ANPR/103031", "Edit");
    headerList << GET_TEXT("ANPR/103032", "Delete");
    ui->tableView_license->setHorizontalHeaderLabels(headerList);
    ui->tableView_license->setColumnCount(headerList.size());
    ui->tableView_license->setColumnHidden(ColumnCheck, true);
    connect(ui->tableView_license, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    //delegate
    ui->tableView_license->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView_license->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //sort
    ui->tableView_license->setSortingEnabled(false);

    //
    m_anprThread = new AnprThread();
    connect(m_anprThread, SIGNAL(importError(QString)), this, SLOT(onImportError(QString)));
    connect(m_anprThread, SIGNAL(importDealRepeat()), this, SLOT(onImportDealRepeat()));
    connect(m_anprThread, SIGNAL(importProgress(int)), this, SLOT(onImportProgress(int)));
    connect(m_anprThread, SIGNAL(importResult(int, int, int, bool, bool)), this, SLOT(onImportResult(int, int, int, bool, bool)));
    connect(m_anprThread, SIGNAL(deleteProgress(int)), this, SLOT(onDeleteProgress(int)));
    connect(m_anprThread, SIGNAL(deleteResult()), this, SLOT(onDeleteResult()));

    m_progressDialog = new ProgressDialog(this);
    connect(m_progressDialog, SIGNAL(sig_cancel()), this, SLOT(onCancelClicked()));

    //
    onLanguageChanged();
}

TabAnprListManagement::~TabAnprListManagement()
{
    if (m_anprThread) {
        m_anprThread->stopThread();
        delete m_anprThread;
    }
    delete ui;
}

void TabAnprListManagement::initializeData(int channel)
{
    Q_UNUSED(channel)

    readLicense();
}

void TabAnprListManagement::showEvent(QShowEvent *)
{
    int w = ui->tableView_license->width() / 4;
    ui->tableView_license->setColumnWidth(ColumnLicense, w);
    ui->tableView_license->setColumnWidth(ColumnType, w);
    ui->tableView_license->setColumnWidth(ColumnEdit, w);
}

void TabAnprListManagement::resetAndReadLicense()
{
    ui->lineEdit_licensePlate->clear();
    ui->comboBox_licenseType->setCurrentIndexFromData("All");
    m_allLicenseMap.clear();
    readLicense();
}

void TabAnprListManagement::readLicense()
{
    const QString &strPlate = ui->lineEdit_licensePlate->text();
    const QString &strType = ui->comboBox_licenseType->currentData().toString();
    m_searchedLicenseList.clear();

    int licenseCount = 0;
    anpr_list *license_array = nullptr;
    read_anpr_lists(SQLITE_ANPR_NAME, &license_array, &licenseCount);
    qDebug() << QString("AnprListManagementPage::initializeData, license count: %1").arg(licenseCount);
    for (int i = 0; i < licenseCount; ++i) {
        const anpr_list &license = license_array[i];
        m_allLicenseMap.insert(QString(license.plate), license);

        if (strPlate.isEmpty() || QString(license.plate).contains(strPlate, Qt::CaseInsensitive)) {
            if (strType == QString("All") || (strType == QString(PARAM_MS_ANPR_TYPE_BLACK) && QString(license.type) == QString(PARAM_MS_ANPR_TYPE_BLACK)) || (strType == QString(PARAM_MS_ANPR_TYPE_WHITE) && QString(license.type) == QString(PARAM_MS_ANPR_TYPE_WHITE))) {
                m_searchedLicenseList.append(license);
            }
        }
    }
    release_anpr_lists(&license_array);

    m_pageIndex = 0;
    m_pageCount = qCeil(m_searchedLicenseList.size() / 100.0);
    if (m_pageCount < 1) {
        m_pageCount = 1;
    }
    ui->label_count->setText(GET_TEXT("LOG/64100", "Total %1 Items").arg(m_searchedLicenseList.size()));
    ui->lineEdit_page->setMaxPage(m_pageCount);
    updateLicenseTable();
}

void TabAnprListManagement::updateLicenseTable()
{
    ui->tableView_license->clearContent();
    ui->tableView_license->scrollToTop();

    int row = 0;
    for (int i = m_searchedLicenseList.size() - 1 - m_pageIndex * 100; i >= 0; --i) {
        const anpr_list &license = m_searchedLicenseList.at(i);
        ui->tableView_license->setItemText(row, ColumnLicense, QString(license.plate));
        ui->tableView_license->setItemText(row, ColumnType, QString(license.type));
        row++;
        if (row >= 100) {
            break;
        }
    }
    ui->label_page->setText(GET_TEXT("LAYOUT/40004", "Page: %1~%2").arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));
}

void TabAnprListManagement::onLanguageChanged()
{
    ui->label_licensePlate->setText(GET_TEXT("ANPR/103035", "License Plate"));
    ui->label_licenseType->setText(GET_TEXT("ANPR/103041", "Plate Type"));

    ui->pushButton_search->setText(GET_TEXT("ANPR/103037", "Search"));
    ui->pushButton_go->setText(GET_TEXT("PLAYBACK/80065", "Go"));
    ui->pushButton_template->setText(GET_TEXT("ANPR/103042", "Download Template"));
    ui->pushButton_add->setText(GET_TEXT("ANPR/103043", "Add"));
    ui->pushButton_deleteList->setText(GET_TEXT("ANPR/103044", "Delete List"));
    ui->pushButton_import->setText(GET_TEXT("ANPR/103049", "Import"));
    ui->pushButton_export->setText(GET_TEXT("ANPR/103052", "Export"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabAnprListManagement::onTableItemClicked(int row, int column)
{
    switch (column) {
    case ColumnEdit: {
        const QString &strPlate = ui->tableView_license->itemText(row, ColumnLicense);
        const QString &strType = ui->tableView_license->itemText(row, ColumnType);

        LicensePlateEdit licenseEdit(this);
        licenseEdit.setLicensePlate(strPlate, strType);
        int result = licenseEdit.exec();
        if (result == LicensePlateEdit::Accepted) {
            resetAndReadLicense();
        }
        break;
    }
    case ColumnDelete: {
        int result = MessageBox::question(this, GET_TEXT("ANPR/103048", "Do you really want to delete present list?"));
        if (result == MessageBox::Yes) {
            const QString &strPlate = ui->tableView_license->itemText(row, ColumnLicense);
            const QString &strType = ui->tableView_license->itemText(row, ColumnType);

            anpr_list anpr_info;
            memset(&anpr_info, 0, sizeof(anpr_list));
            snprintf(anpr_info.plate, sizeof(anpr_info.plate), "%s", strPlate.toStdString().c_str());
            snprintf(anpr_info.type, sizeof(anpr_info.type), "%s", strType.toStdString().c_str());
            delete_anpr_list(SQLITE_ANPR_NAME, &anpr_info);

            resetAndReadLicense();
        }
        break;
    }
    default:
        break;
    }
}

void TabAnprListManagement::onImportError(const QString &error)
{
    if (m_progressDialog) {
        m_progressDialog->hideProgress();
    }
    ShowMessageBox(error);
}

void TabAnprListManagement::onImportDealRepeat()
{
    int result = MessageBox::question(this, GET_TEXT("ANPR/103045", "Detected the same plate(s). Do you want to replace the old ones?"));
    if (result == MessageBox::Cancel) {
        m_anprThread->setImportRepeatMode(AnprThread::ModeIgnore);
    } else {
        m_anprThread->setImportRepeatMode(AnprThread::ModeReplace);
    }
}

void TabAnprListManagement::onImportProgress(int progress)
{
    if (m_progressDialog) {
        m_progressDialog->setProgressValue(progress);
    }
}

void TabAnprListManagement::onImportResult(int succeedCount, int failedCount, int errorCount, bool isReachLimit, bool isCancel)
{
    if (m_progressDialog) {
        m_progressDialog->hideProgress();
    }
    resetAndReadLicense();
    if (isReachLimit) {
        ShowMessageBox(QString(GET_TEXT("ANPR/103046", "Out of capacity！Successful:%1 Failed:%2")).arg(succeedCount).arg(failedCount));
    } else if (errorCount > 0) {
        ShowMessageBox(QString(GET_TEXT("ANPR/103051", "Content error!  Successful:%1 Failed:%2")).arg(succeedCount).arg(failedCount));
    } else if (isCancel) {
        ShowMessageBox(QString("Import cancel!  Successful:%1").arg(succeedCount));
    }
}

void TabAnprListManagement::onDeleteProgress(int progress)
{
    if (m_progressDialog) {
        m_progressDialog->setProgressValue(progress);
    }
}

void TabAnprListManagement::onDeleteResult()
{
    if (m_progressDialog) {
        m_progressDialog->hideProgress();
    }
    resetAndReadLicense();
}

void TabAnprListManagement::onCancelClicked()
{
    m_anprThread->stopOperate();
}

void TabAnprListManagement::on_toolButton_firstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    updateLicenseTable();
}

void TabAnprListManagement::on_toolButton_previousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    updateLicenseTable();
}

void TabAnprListManagement::on_toolButton_nextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    updateLicenseTable();
}

void TabAnprListManagement::on_toolButton_lastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    updateLicenseTable();
}

void TabAnprListManagement::on_pushButton_go_clicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (m_pageIndex == page - 1) {
        return;
    }
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;
    updateLicenseTable();
}

void TabAnprListManagement::on_pushButton_search_clicked()
{
    readLicense();
}

void TabAnprListManagement::on_pushButton_add_clicked()
{
    int anprListCount = read_anpr_list_cnt(SQLITE_ANPR_NAME);
    if (anprListCount >= MAX_ANPR_LIST_COUNT) {
        ShowMessageBox(GET_TEXT("ANPR/103065", "Out of capacity！"));
        return;
    }
    //
    LicensePlateAdd plateAdd(this);
    int result = plateAdd.exec();
    if (result == LicensePlateAdd::Accepted) {
        resetAndReadLicense();
    }
}

void TabAnprListManagement::on_pushButton_deleteList_clicked()
{
    const int result = MessageBox::question(this, GET_TEXT("ANPR/103048", "Do you really want to delete present list?"));
    if (result == MessageBox::Cancel) {
        return;
    }
    m_anprThread->deleteAnprList(m_searchedLicenseList);

    m_progressDialog->showProgress();
}

void TabAnprListManagement::on_pushButton_import_clicked()
{
    if (m_allLicenseMap.size() >= MAX_ANPR_LIST_COUNT) {
        ShowMessageBox(GET_TEXT("ANPR/103065", "Out of capacity！"));
        return;
    }

    const QString &strFilePath = MyFileSystemDialog::instance()->getOpenFileInfo();
    if (strFilePath.isEmpty()) {
        return;
    }

    m_anprThread->importAnprList(strFilePath);

    m_progressDialog->showProgress();
}

void TabAnprListManagement::on_pushButton_export_clicked()
{
    const QString &strPath = MyFileSystemDialog::instance()->getOpenDirectory();
    if (strPath.isEmpty()) {
        return;
    }
    const qint64 &deviceFreeBytes = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    if ((deviceFreeBytes >> 20) < MIN_EXPORT_DISK_FREE) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

    QString fileName = QString("anpr_list_%2.csv").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    QFile file(QString("%1/%2").arg(strPath).arg(fileName));

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        ShowMessageBox(file.errorString());
        return;
    }
    QTextStream out(&file);
    out << QString("Type,Plate\n");
    for (int i = 0; i < m_searchedLicenseList.size(); ++i) {
        const anpr_list &anpr_info = m_searchedLicenseList.at(i);
        out << QString("%1,%2\n").arg(QString(anpr_info.type)).arg(QString(anpr_info.plate));
    }
    file.close();
    sync();
    ShowMessageBox(GET_TEXT("ANPR/103074", "Download completed, file name: %1").arg(fileName));
}

void TabAnprListManagement::on_pushButton_back_clicked()
{
    back();
}

void TabAnprListManagement::on_pushButton_template_clicked()
{
    const QString &strPath = MyFileSystemDialog::instance()->getOpenDirectory();
    if (strPath.isEmpty()) {
        return;
    }
    QFile file(QString("%1/list_demo.csv").arg(strPath));

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        ShowMessageBox(file.errorString());
        return;
    }
    QTextStream out(&file);
    out << QString("Type,Plate\n");
    out << QString("White,2008ZGZ\n");
    out << QString("Black,34AB1234\n");
    file.close();
    sync();
    ShowMessageBox(GET_TEXT("ANPR/103074", "Download completed, file name: %1").arg("list_demo.csv"));
}
