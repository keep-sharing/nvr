#include "DownloadPanel.h"
#include "ui_DownloadPanel.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "DownloadButton.h"
#include "DownloadItem.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyFileSystemDialog.h"
#include <QPainter>
#include "MessageBox.h"

DownloadPanel *DownloadPanel::self = nullptr;

DownloadPanel::DownloadPanel(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::DownloadPanel)
{
    ui->setupUi(this);

    self = this;
    m_download = static_cast<DownloadButton *>(parent);

    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    m_timerRemain = new QTimer(this);
    connect(m_timerRemain, SIGNAL(timeout()), this, SLOT(onTimerRemainTime()));
    m_timerRemain->setInterval(1000);

    m_timerSpeed = new QTimer(this);
    connect(m_timerSpeed, SIGNAL(timeout()), this, SLOT(onTimerSpeed()));
    m_timerSpeed->setInterval(1000);
}

DownloadPanel::~DownloadPanel()
{
    self = nullptr;
    delete ui;
}

DownloadPanel *DownloadPanel::instance()
{
    return self;
}

quint32 DownloadPanel::availableId()
{
    if (m_id > (quint32)0x0FFFFFFF) {
        m_id = DOWNLOAD_BEGIN_ID;
    }
    m_id++;
    return m_id;
}

void DownloadPanel::appendItem(const req_auto_backup &backup)
{
    m_backupQueue.enqueue(backup);

    //
    QListWidgetItem *item = new QListWidgetItem();
    ui->listWidget->insertItem(0, item);

    DownloadItem *itemWidget = new DownloadItem(this);
    connect(itemWidget, SIGNAL(deleted(quint32)), this, SLOT(onItemDeleted(quint32)));
    QString name;
    QStringList nameList = QString(backup.filename).split(".");
    if (!nameList.isEmpty()) {
        name = nameList.first();
    } else {
        name = QString(backup.filename);
    }
    itemWidget->initializeData(backup.sid, name);

    ui->listWidget->setItemWidget(item, itemWidget);
    m_itemsMap.insert(backup.sid, itemWidget);
    updateItemWidgetInfo();

    //
    if (!m_isDownloading) {
        m_isDownloading = true;
        m_download->startMovie();
        m_timerRemain->start();
        QTimer::singleShot(100, this, SLOT(downloadNext()));
    }
}

bool DownloadPanel::isDownloading() const
{
    return m_isDownloading;
}

void DownloadPanel::stopDownload()
{
    on_toolButtonDelete_clicked();
}

void DownloadPanel::dealMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_RETRIEVE_EXPORT:
        ON_RESPONSE_FLAG_PROGRESS_RETRIEVE_EXPORT(message);
        break;
    }
}

void DownloadPanel::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_ADD_AUTO_BACKUP:
        ON_RESPONSE_FLAG_ADD_AUTO_BACKUP(message);
        break;
    case RESPONSE_FLAG_UPDATE_AUTO_BACKUP:
        ON_RESPONSE_FLAG_UPDATE_AUTO_BACKUP(message);
        break;
    }
}

void DownloadPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect shadowRect = rect();
    shadowRect.setLeft(m_marginLeft);
    shadowRect.setTop(m_marginTop);
    shadowRect.setRight(width() - m_marginRight);
    shadowRect.setBottom(height() - m_marginBottom);
    shadowRect.adjust(2, 2, -2, -2);

    static int shadowWidth = 9;
    qreal alphaStep = 100.0 / shadowWidth;
    painter.setBrush(Qt::NoBrush);
    QColor color(0, 0, 0, 50);
    for (int i = 0; i < shadowWidth; i++) {
        shadowRect.adjust(-1, -1, 1, 1);
        //int alpha = 150 - qSqrt(i) * 50;
        color.setAlpha(100 - i * alphaStep);
        painter.setPen(color);
        painter.drawRoundedRect(shadowRect, i, i);
    }

    //
    static const QPoint points[7] = {
        QPoint(m_marginLeft, m_marginTop),
        QPoint(m_marginLeft + width() / 3 - 8, m_marginTop),
        QPoint(m_marginLeft + width() / 3, m_marginTop - 10),
        QPoint(m_marginLeft + width() / 3 + 8, m_marginTop),
        QPoint(width() - m_marginRight, m_marginTop),
        QPoint(width() - m_marginRight, height() - m_marginBottom),
        QPoint(m_marginLeft, height() - m_marginBottom)
    };
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(57, 184, 228));
    painter.drawPolygon(points, 7);
}

void DownloadPanel::ON_RESPONSE_FLAG_ADD_AUTO_BACKUP(MessageReceive *message)
{
    resp_auto_backup *backup = static_cast<resp_auto_backup *>(message->data);
    if (backup) {
        qMsDebug() << QString("id: %1, state: %2").arg(backup->sid).arg(backup->state);
        if (backup->state == 0) {
            //成功
            if (m_itemsMap.contains(backup->sid)) {
                DownloadItem *item = m_itemsMap.value(backup->sid);
                item->setProgress(100);
                item->setCompleted();
            }
        } else {
            //失败
            if (m_itemsMap.contains(backup->sid)) {
                DownloadItem *item = m_itemsMap.value(backup->sid);
                item->setErrorVisible(true);
                item->setCompleted();
                //
                updateDownloadState();
            }
        }
    } else {
        qMsWarning() << message;
    }
    downloadNext();
}

void DownloadPanel::ON_RESPONSE_FLAG_PROGRESS_RETRIEVE_EXPORT(MessageReceive *message)
{
    PROGRESS_BAR_T *progress = (PROGRESS_BAR_T *)message->data;
    if (progress) {
        //qMsWarning() << QString("sid: %1, percent: %2%").arg(progress->sid).arg(progress->percent);
        if (m_itemsMap.contains(progress->sid)) {
            DownloadItem *item = m_itemsMap.value(progress->sid);
            item->setProgress(progress->percent);
            //
            quint64 size = backupSize(m_currentBackup);
            m_currentRemainSize = (100 - progress->percent) / 100.0 * size;
            if (size > 0) {
                int sec = m_speedSec;
                if (sec > 0) {
                    m_speed = progress->percent / 100.0 * size / sec;
                    qMsCDebug("qt_download_speed") << QString("Download Speed: %1KB/s").arg(m_speed / 1024);
                    updateRemainSec();
                }
            }
        }
    } else {
        qMsWarning() << message;
    }
}

void DownloadPanel::ON_RESPONSE_FLAG_UPDATE_AUTO_BACKUP(MessageReceive *message)
{
    int result = *(int *)message->data;
    Q_UNUSED(result)
}

void DownloadPanel::updateItemWidgetInfo()
{
    m_itemsMap.clear();
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        DownloadItem *itemWidget = static_cast<DownloadItem *>(ui->listWidget->itemWidget(item));
        itemWidget->setRow(i);
        m_itemsMap.insert(itemWidget->id(), itemWidget);
    }
}

void DownloadPanel::updateDownloadState()
{
    bool hasError = false;
    for (auto iter = m_itemsMap.constBegin(); iter != m_itemsMap.constEnd(); ++iter) {
        DownloadItem *item = iter.value();
        if (item->hasError()) {
            hasError = true;
            break;
        }
    }
    m_download->setErrorVisible(hasError);
}

void DownloadPanel::updateRemainSec()
{
    quint64 size = 0;
    int otherSec = 0;
    for (int i = 0; i < m_backupQueue.size(); ++i) {
        const req_auto_backup &backup = m_backupQueue.at(i);
        size += backupSize(backup);
    }
    //当前下载文件大小
    size += m_currentRemainSize;

    m_remainSec = 0;
    if (m_speed > 0) {
        m_remainSec = size / m_speed;
    }
    m_remainSec += otherSec;
}

void DownloadPanel::removeFromQueue(quint32 id)
{
    for (int i = 0; i < m_backupQueue.size(); ++i) {
        const req_auto_backup &backup = m_backupQueue.at(i);
        if (id == backup.sid) {
            m_backupQueue.removeAt(i);
            break;
        }
    }
}

quint64 DownloadPanel::backupSize(const req_auto_backup &backup) const
{
    quint64 size = 0;
    if (backup.filetype == 3) {
        //图片文件按2秒算
        size += 5 * 1024 * 1024;
    } else {
        if (backup.filesize == 0) {
            //按照1分钟30M算，1秒0.5M
            QDateTime begin = QDateTime::fromString(backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
            QDateTime end = QDateTime::fromString(backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
            size += begin.secsTo(end) * 0.5 * 1024 * 1024;
        } else {
            size += backup.filesize;
        }
    }
    return size;
}

void DownloadPanel::onLanguageChanged()
{
    ui->labelRemainingTime->setText(GET_TEXT("DOWNLOAD/60100", "Remaining Time"));
}

void DownloadPanel::downloadNext()
{
    m_currentBackup.sid = 0;
    updateRemainSec();
    //
    if (m_backupQueue.isEmpty()) {
        m_isDownloading = false;
        m_download->stopMovie();
        m_timerRemain->stop();
        ui->labelTime->setText("00:00:00");
        QString text = GET_TEXT("DOWNLOAD/173000", "Export successfully!");
        QMetaObject::invokeMethod(MessageBox::instance(), "showInformation", Qt::QueuedConnection, Q_ARG(QString, text));
    } else {
        m_currentBackup = m_backupQueue.dequeue();
        qMsDebug() << QString("id: %1, dev_path: %2, filename: %3").arg(m_currentBackup.sid).arg(m_currentBackup.dev_path).arg(m_currentBackup.filename);
        sendMessage(REQUEST_FLAG_ADD_AUTO_BACKUP, (void *)&m_currentBackup, sizeof(struct req_auto_backup));
        m_speedSec = 0;
        m_timerSpeed->start();
    }
}

void DownloadPanel::onItemDeleted(quint32 id)
{
    bool isQueue = false;
    for (int i = 0; i < m_backupQueue.size(); ++i) {
        const req_auto_backup &backup = m_backupQueue.at(i);
        if (backup.sid == id) {
            isQueue = true;
            break;
        }
    }
    if (isQueue) {
        //删除下载队列中的
        removeFromQueue(id);
    } else if (id == m_currentBackup.sid) {
        //删除正在下载的
        req_auto_backup_state state;
        state.sid = id;
        state.state = 1;
        qMsDebug() << "REQUEST_FLAG_UPDATE_AUTO_BACKUP, sid:" << id;
        Q_UNUSED(state)
        m_currentBackup.sid = 0;
        m_currentRemainSize = 0;
    } else {
        //删除下载完成的
    }
    //
    DownloadItem *itemWidget = m_itemsMap.value(id);
    int row = itemWidget->row();
    QListWidgetItem *item = ui->listWidget->takeItem(row);
    delete item;
    m_itemsMap.remove(id);
    //
    updateItemWidgetInfo();
    updateDownloadState();
    updateRemainSec();
    //
    if (m_backupQueue.isEmpty() && m_currentBackup.sid == 0) {
        m_isDownloading = false;
        m_download->stopMovie();
        m_timerRemain->stop();
        ui->labelTime->setText("00:00:00");
    }
}

void DownloadPanel::onTimerRemainTime()
{
    m_remainSec -= 1;
    if (m_remainSec < 0) {
        m_remainSec = 0;
    }
    if (m_remainSec == 0 && m_isDownloading) {
        m_remainSec = 1;
    }
    //
    int hour = m_remainSec / 3600;
    int minute = m_remainSec % 3600 / 60;
    int second = m_remainSec % 60;
    ui->labelTime->setText(QString("%1:%2:%3")
                               .arg(hour, 2, 10, QLatin1Char('0'))
                               .arg(minute, 2, 10, QLatin1Char('0'))
                               .arg(second, 2, 10, QLatin1Char('0')));
}

void DownloadPanel::onTimerSpeed()
{
    m_speedSec++;
}

void DownloadPanel::on_toolButtonDelete_clicked()
{
    if (m_isDownloading) {
        //删除正在下载的
        req_auto_backup_state state;
        state.sid = m_currentBackup.sid;
        state.state = 1;
        Q_UNUSED(state)
        m_currentBackup.sid = 0;
    }
    ui->listWidget->clear();
    m_backupQueue.clear();
    m_itemsMap.clear();
    m_isDownloading = false;
    m_download->stopMovie();
    updateDownloadState();
    m_remainSec = 0;
    onTimerRemainTime();
    m_timerRemain->stop();
}

void DownloadPanel::on_toolButtonUDisk_clicked()
{
    MyFileSystemDialog::instance()->showDialog();
}
