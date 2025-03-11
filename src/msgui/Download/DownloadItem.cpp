#include "DownloadItem.h"
#include "ui_DownloadItem.h"
#include "MyDebug.h"

DownloadItem::DownloadItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadItem)
{
    ui->setupUi(this);
    ui->labelWarning->hide();
}

DownloadItem::~DownloadItem()
{
    delete ui;
}

void DownloadItem::initializeData(quint32 id, const QString &name)
{
    m_id = id;
    ui->labelName->setElidedText(name);
    ui->progressBar->setValue(0);
    ui->labelValue->setText(QString("0%"));
}

void DownloadItem::setProgress(int value)
{
    ui->progressBar->setValue(value);
    ui->labelValue->setText(QString("%1%").arg(value));
}

void DownloadItem::setCompleted()
{
    m_isCompleted = true;
}

bool DownloadItem::isCompleted()
{
    return m_isCompleted;
}

void DownloadItem::setErrorVisible(bool visible)
{
    m_showError = visible;

    ui->labelWarning->setVisible(visible);

    ui->progressBar->setProperty("error", visible);
    ui->progressBar->style()->polish(ui->progressBar);
}

bool DownloadItem::hasError() const
{
    return m_showError;
}

quint32 DownloadItem::id() const
{
    return m_id;
}

void DownloadItem::setRow(int row)
{
    m_row = row;
}

int DownloadItem::row() const
{
    return m_row;
}

void DownloadItem::on_toolButtonClose_clicked()
{
    emit deleted(m_id);
}
