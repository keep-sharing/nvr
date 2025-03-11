#include "AbstractPlaybackFile.h"
#include "ui_AbstractPlaybackFile.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QtDebug>

QMap<int, int> AbstractPlaybackFile::s_tempSidMap;

AbstractPlaybackFile::AbstractPlaybackFile(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AbstractPlaybackFile)
{
    ui->setupUi(this);

    connect(ui->tableView, SIGNAL(headerChecked(bool)), this, SLOT(onTableHeaderClicked(bool)));
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onItemClicked(int, int)));

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

AbstractPlaybackFile::~AbstractPlaybackFile()
{
    delete ui;
}

void AbstractPlaybackFile::clear()
{
}

bool AbstractPlaybackFile::hasFreeSid()
{
    if (s_tempSidMap.isEmpty()) {
        return true;
    }
    auto iter = s_tempSidMap.end();
    iter--;
    int sid = iter.key();
    if (sid >= 63) {
        return false;
    }
    return true;
}

void AbstractPlaybackFile::closeAllSid()
{
    for (auto iter = s_tempSidMap.constBegin(); iter != s_tempSidMap.constEnd(); ++iter) {
        int sid = iter.key();
        qDebug() << QString("AbstractPlaybackFile::closeAllSid, sid: %1").arg(sid);
        MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_SEARCH_COM_BACKUP_CLOSE, &sid, sizeof(int));
    }
    s_tempSidMap.clear();
}

void AbstractPlaybackFile::dealSelected()
{
}

void AbstractPlaybackFile::updateTableList()
{
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1        Page: %2/%3")
                                .arg(itemCount())
                                .arg(currentPage())
                                .arg(pageCount()));
    ui->lineEdit_page->setText(QString::number(currentPage()));
}

/**
 * @brief AbstractPlaybackFile::currentPage
 * @return 1-n，pageCount为0时返回0
 */
int AbstractPlaybackFile::currentPage()
{
    if (pageCount() > 0) {
        return m_pageIndex + 1;
    } else {
        return 0;
    }
}

void AbstractPlaybackFile::onLanguageChanged()
{
    ui->toolButton_firstPage->setToolTip(GET_TEXT("PLAYBACK/80061", "First Page"));
    ui->toolButton_previousPage->setToolTip(GET_TEXT("PLAYBACK/80062", "Previous Page"));
    ui->toolButton_nextPage->setToolTip(GET_TEXT("PLAYBACK/80063", "Next Page"));
    ui->toolButton_lastPage->setToolTip(GET_TEXT("PLAYBACK/80064", "Last Page"));
    ui->pushButton_go->setText(GET_TEXT("PLAYBACK/80065", "Go"));
    ui->label_selectedCount->setText(QString("%1 %2").arg(GET_TEXT("PLAYBACK/80106", "Selected Items:")).arg(QString::number(0)));
    ui->label_selectedSize->setText(QString("%1 %2").arg(GET_TEXT("PLAYBACK/80107", "Total Size:")).arg(TableView::bytesString(0)));
}

void AbstractPlaybackFile::onTableHeaderClicked(bool checked)
{
    Q_UNUSED(checked)

    dealSelected();
}

void AbstractPlaybackFile::onItemClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
}

void AbstractPlaybackFile::on_toolButton_firstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    updateTableList();
}

void AbstractPlaybackFile::on_toolButton_previousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    updateTableList();
}

void AbstractPlaybackFile::on_toolButton_nextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    updateTableList();
}

void AbstractPlaybackFile::on_toolButton_lastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    updateTableList();
}

void AbstractPlaybackFile::on_pushButton_go_clicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;
    updateTableList();
}
