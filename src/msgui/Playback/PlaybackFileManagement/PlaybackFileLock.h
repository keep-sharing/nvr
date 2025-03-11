#ifndef PLAYBACKFILELOCK_H
#define PLAYBACKFILELOCK_H

#include "AbstractPlaybackFile.h"
#include "MsWaitting.h"
#include <QEventLoop>

class PlaybackFileLock : public AbstractPlaybackFile {
    Q_OBJECT

    enum ColumnType {
        ColumnCheck,
        ColumnChannel,
        ColumnTime,
        ColumnSize,
        ColumnLock
    };

public:
    explicit PlaybackFileLock(QWidget *parent = nullptr);

    void updateTableList() override;
    void clear() override;

    int addLockFile(const resp_search_common_backup &common_backup);
    int waitForLockAll(const QDateTime &dateTime);
    int waitForLock(const QDateTime &dateTime);

    //export
    void exportLock();
    void exportAllLock();

    void dealMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_LOCK_COM_BACKUP(MessageReceive *message);

protected:
    void onLanguageChanged();

    virtual int pageCount() override;
    virtual int itemCount() override;
    virtual void dealSelected() override;
    //table
    virtual QStringList tableHeaders() override;
    virtual void initializeTableView() override;

protected slots:
    virtual void onItemClicked(int row, int column) override;

private:
    QMap<PlaybackFileKey, resp_search_common_backup> m_fileMap;
    QEventLoop m_eventLoop;

    MsWaitting *m_waitting = nullptr;

    QList<resp_search_common_backup> m_search_common_backup_list;
};

#endif // PLAYBACKFILELOCK_H
