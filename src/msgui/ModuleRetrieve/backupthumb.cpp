#include "backupthumb.h"
#include "ui_backupthumb.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyFileModel.h"
#include "centralmessage.h"
#include <QtDebug>
#include <qmath.h>

BackupThumb::BackupThumb(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::BackupThumb)
{
    ui->setupUi(this);

    //
    for (int row = 0; row < 4; ++row) {
        ui->gridLayout_item->setRowStretch(row, 1);
    }
    for (int column = 0; column < 5; ++column) {
        ui->gridLayout_item->setColumnStretch(column, 1);
    }
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 5; ++column) {
            int index = row * 5 + column;
            AnprItemWidget *item = new AnprItemWidget(this);
            item->setIndex(index);
            item->setInfoVisible(false);
            connect(item, SIGNAL(itemClicked(int)), this, SLOT(onItemClicked(int)));
            connect(item, SIGNAL(itemChecked(int, bool)), this, SLOT(onItemChecked(int, bool)));
            ui->gridLayout_item->addWidget(item, row, column);

            m_itemList.append(item);
        }
    }

    m_waitting = new MsWaitting(this);

    onLanguageChanged();
}

BackupThumb::~BackupThumb()
{
    delete ui;
}

void BackupThumb::setCommonBackupList(const QList<resp_search_common_backup> &backup_list)
{
    m_mode = ModeCommon;

    m_commonBackupList = backup_list;
    if (m_allBackupCount != m_commonBackupList.size()) {
        m_allBackupCount = m_commonBackupList.size();
        m_pageCount = qCeil(m_allBackupCount / 20.0);
        m_pageIndex = 0;
    }
    ui->lineEdit_page->setMaxPage(m_pageCount);

    if (!m_commonBackupList.isEmpty()) {
        m_allBytes = m_commonBackupList.first().allSize;
    } else {
        m_allBytes = 0;
    }

    updateTable();
    updateCheckedSize();
}

void BackupThumb::setEventBackupList(const QList<resp_search_event_backup> &backup_list)
{
    m_mode = ModeEvent;

    m_eventBackupList = backup_list;
    if (m_allBackupCount != m_eventBackupList.size()) {
        m_allBackupCount = m_eventBackupList.size();
        m_pageCount = qCeil(m_allBackupCount / 20.0);
        m_pageIndex = 0;
    }
    ui->lineEdit_page->setMaxPage(m_pageCount);

    if (!m_eventBackupList.isEmpty()) {
        m_allBytes = m_eventBackupList.first().allSize;
    } else {
        m_allBytes = 0;
    }

    updateTable();
    updateCheckedSize();
}

QList<resp_search_common_backup> BackupThumb::checkedCommonBackupList() const
{
    QList<resp_search_common_backup> backupList;

    int backupIndex = m_pageIndex * 20;
    for (int itemIndex = 0; itemIndex < 20; ++itemIndex) {
        AnprItemWidget *item = m_itemList.at(itemIndex);
        if (item->isChecked() && m_commonBackupList.size() > backupIndex) {
            const resp_search_common_backup &backup = m_commonBackupList.at(backupIndex);
            backupList.append(backup);
        }
        backupIndex++;
    }

    return backupList;
}

QList<resp_search_event_backup> BackupThumb::checkedEventBackupList() const
{
    QList<resp_search_event_backup> backupList;

    int backupIndex = m_pageIndex * 20;
    for (int itemIndex = 0; itemIndex < 20; ++itemIndex) {
        AnprItemWidget *item = m_itemList.at(itemIndex);
        if (item->isChecked() && m_eventBackupList.size() > backupIndex) {
            const resp_search_event_backup &backup = m_eventBackupList.at(backupIndex);
            backupList.append(backup);
        }
        backupIndex++;
    }

    return backupList;
}

resp_search_common_backup BackupThumb::selectedCommonBakcup() const
{
    resp_search_common_backup backup;
    memset(&backup, 0, sizeof(resp_search_common_backup));
    if (m_selectedBackupIndex < 0) {
        backup.chnid = -1;
    } else {
        const resp_search_common_backup &temp_backup = m_commonBackupList.at(m_selectedBackupIndex);
        memcpy(&backup, &temp_backup, sizeof(resp_search_common_backup));
    }
    return backup;
}

resp_search_event_backup BackupThumb::selectedEventBakcup() const
{
    resp_search_event_backup backup;
    memset(&backup, 0, sizeof(resp_search_event_backup));
    if (m_selectedBackupIndex < 0) {
        backup.chnid = -1;
    } else {
        const resp_search_event_backup &temp_backup = m_eventBackupList.at(m_selectedBackupIndex);
        memcpy(&backup, &temp_backup, sizeof(resp_search_event_backup));
    }
    return backup;
}

QImage BackupThumb::selectedImage() const
{
    AnprItemWidget *item = m_itemList.at(m_selectedItemIndex);
    return item->image();
}

void BackupThumb::selectNext()
{
    if (m_selectedItemIndex < m_itemCountInPage - 1) {
        int index = m_selectedItemIndex + 1;
        AnprItemWidget *item = m_itemList.at(index);
        if (item->isInfoVisible()) {
            onItemClicked(index);
        }
    }
}

void BackupThumb::selectPrevious()
{
    if (m_selectedItemIndex > 0) {
        int index = m_selectedItemIndex - 1;
        AnprItemWidget *item = m_itemList.at(index);
        if (item->isInfoVisible()) {
            onItemClicked(index);
        }
    }
}

void BackupThumb::clearTable()
{
    for (int i = 0; i < m_itemList.size(); ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        item->setInfoVisible(false);
        item->setSelected(false);
        item->setChecked(false);
    }
    ui->checkBox_all->setChecked(false);
}

void BackupThumb::updateTable()
{
    //qDebug() << QString("BackupThumb::updateTable, index: %1").arg(m_pageIndex);

    ui->label_size->setText(QString("%1 %2").arg(GET_TEXT("COMMONBACKUP/100050", "Total Size: ")).arg(MyFileModel::fileSize(m_allBytes)));
    ui->label_page->setText(GET_TEXT("COMMONBACKUP/100011", "Total: %1          Page: %2/%3").arg(m_allBackupCount).arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));

    //m_waitting->//showWait();

    ui->checkBox_all->setChecked(false);
    clearTable();
    m_showItemCount = 0;
    int backupIndex = m_pageIndex * 20;
    for (int itemIndex = 0; itemIndex < 20; ++itemIndex) {
        AnprItemWidget *item = m_itemList.at(itemIndex);
        switch (m_mode) {
        case ModeCommon:
            if (m_commonBackupList.size() > backupIndex) {
                const resp_search_common_backup &backup = m_commonBackupList.at(backupIndex);

                struct rep_play_common_backup playinfo;
                memset(&playinfo, 0, sizeof(struct rep_play_common_backup));
                playinfo.chnid = backup.chnid;
                playinfo.sid = backup.sid;
                if (qMsNvr->isnt98323() || qMsNvr->isnt98633()) {
                    playinfo.flag = 1;
                } else {
                    playinfo.flag = 0;
                }
                playinfo.enType = StreamType;
                snprintf(playinfo.pStartTime, sizeof(playinfo.pStartTime), "%s", backup.pStartTime);
                snprintf(playinfo.pEndTime, sizeof(playinfo.pEndTime), "%s", backup.pEndTime);
                sendMessage(REQUEST_FLAG_PLAY_COM_PICTURE, (void *)&playinfo, sizeof(struct rep_play_common_backup));

                //m_eventLoop.exec();

                QDateTime dateTime = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
                item->setItemInfo(backup.chnid, dateTime, m_image);
                item->setInfoVisible(true);
                m_showItemCount ++;
            } else {
                item->setInfoVisible(false);
            }
            break;
        case ModeEvent:
            ui->label_size->setText("");
            if (m_eventBackupList.size() > backupIndex) {
                const resp_search_event_backup &backup = m_eventBackupList.at(backupIndex);

                QDateTime begin = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
                QDateTime end = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
                m_control->waitForSearchCommonBackup(backup.chnid, begin, end, StreamType);
                m_control->waitForSearchCommonBackupPicture();
                m_control->waitForCloseCommonBackup();

                item->setItemInfo(backup.chnid, begin, m_control->image());
                item->setInfoVisible(true);
                m_showItemCount ++;
            } else {
                item->setInfoVisible(false);
            }
            break;
        default:
            break;
        }
        backupIndex++;
    }

    //m_waitting->//closeWait();

    onItemClicked(0);
}

void BackupThumb::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PLAY_COM_PICTURE:
        ON_RESPONSE_FLAG_PLAY_COM_PICTURE(message);
        break;
    case RESPONSE_FLAG_PLAY_EVT_PICTURE:
        ON_RESPONSE_FLAG_PLAY_EVT_PICTURE(message);
        break;
    default:
        break;
    }
}

void BackupThumb::ON_RESPONSE_FLAG_PLAY_COM_PICTURE(MessageReceive *message)
{
    if (message->header.size == -1 || message->data) {
        if (message->data) {
            m_image = message->image1;
        } else {
            m_image = QImage(":/retrieve/retrieve/error_image.png");
        }
    }
    if (m_waitting->isVisible()) {
        message->accept();
    }
    m_eventLoop.exit();
}

void BackupThumb::ON_RESPONSE_FLAG_PLAY_EVT_PICTURE(MessageReceive *message)
{
    if (message->header.size == -1 || message->data) {
        if (message->data) {
            m_image = message->image1;
        } else {
            m_image = QImage(":/retrieve/retrieve/error_image.png");
        }
    }
    if (m_waitting->isVisible()) {
        message->accept();
    }
    m_eventLoop.exit();
}

void BackupThumb::updateCheckedSize()
{
    qint64 bytes = 0;

    int backupIndex = m_pageIndex * 20;
    for (int itemIndex = 0; itemIndex < 20; ++itemIndex) {
        AnprItemWidget *item = m_itemList.at(itemIndex);
        switch (m_mode) {
        case ModeCommon:
            if (item->isChecked() && m_commonBackupList.size() > backupIndex) {
                const resp_search_common_backup &backup = m_commonBackupList.at(backupIndex);
                bytes += backup.size;
            }
            break;
        case ModeEvent:
            if (item->isChecked() && m_eventBackupList.size() > backupIndex) {
                const resp_search_event_backup &backup = m_eventBackupList.at(backupIndex);
                bytes += backup.size;
            }
            break;
        default:
            break;
        }
        backupIndex++;
    }

    if (m_mode == ModeCommon) {
        if (bytes > 0) {
            ui->label_size->setText(GET_TEXT("COMMONBACKUP/100037", "Total Size: %1         Select Size: %2").arg(MyFileModel::fileSize(m_allBytes)).arg(MyFileModel::fileSize(bytes)));
        } else {
            ui->label_size->setText(QString("%1 %2").arg(GET_TEXT("COMMONBACKUP/100050", "Total Size: ")).arg(MyFileModel::fileSize(m_allBytes)));
        }
    }
}

void BackupThumb::onLanguageChanged()
{
    ui->label_list->setText(GET_TEXT("RETRIEVE/96002", "Chart"));
    ui->checkBox_all->setText(GET_TEXT("SYSTEMGENERAL/164000", "All"));

    ui->toolButton_firstPage->setToolTip(GET_TEXT("PLAYBACK/80061", "First Page"));
    ui->toolButton_previousPage->setToolTip(GET_TEXT("PLAYBACK/80062", "Previous Page"));
    ui->toolButton_nextPage->setToolTip(GET_TEXT("PLAYBACK/80063", "Next Page"));
    ui->toolButton_lastPage->setToolTip(GET_TEXT("PLAYBACK/80064", "Last Page"));
}

void BackupThumb::onItemClicked(int index)
{
    m_selectedItemIndex = index;
    for (int i = 0; i < 20; ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        if (i == index) {
            item->setSelected(true);
        } else {
            item->setSelected(false);
        }
    }

    m_selectedBackupIndex = m_pageIndex * 20 + index;
    emit itemClicked();
}

void BackupThumb::onItemChecked(int index, bool checked)
{
    Q_UNUSED(index)
    Q_UNUSED(checked)

    int checkedCount = 0;
    for (int i = 0; i < 20; ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        if (item->isChecked()) {
            checkedCount++;
        }
    }
    if (checkedCount == 0) {
        ui->checkBox_all->setCheckState(Qt::Unchecked);
    } else if (checkedCount == m_itemList.size() || checkedCount == m_showItemCount) {
        ui->checkBox_all->setCheckState(Qt::Checked);
    } else {
        ui->checkBox_all->setCheckState(Qt::PartiallyChecked);
    }

    updateCheckedSize();
}

void BackupThumb::on_checkBox_all_clicked(bool checked)
{
    if (ui->checkBox_all->checkState() == Qt::PartiallyChecked) {
        ui->checkBox_all->setCheckState(Qt::Checked);
    }

    for (int i = 0; i < 20; ++i) {
        AnprItemWidget *item = m_itemList.at(i);
        if (item->isInfoVisible()) {
            item->setChecked(checked);
        }
    }
    updateCheckedSize();
}

void BackupThumb::on_toolButton_firstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;

    emit stopPlay();

    updateTable();
}

void BackupThumb::on_toolButton_previousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;

    emit stopPlay();

    updateTable();
}

void BackupThumb::on_toolButton_nextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;

    emit stopPlay();

    updateTable();
}

void BackupThumb::on_toolButton_lastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;

    emit stopPlay();

    updateTable();
}

void BackupThumb::on_pushButton_go_clicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;

    emit stopPlay();

    updateTable();
}

void BackupThumb::setPageIndex(int pageIndex)
{
    m_pageIndex = pageIndex;
}

void BackupThumb::setControl(CommonBackupControl *newControl)
{
    m_control = newControl;
}
