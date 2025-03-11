#ifndef PLAYBACKFILESNAPSHOT_H
#define PLAYBACKFILESNAPSHOT_H

#include "AbstractPlaybackFile.h"
#include <QEventLoop>

class PlaybackFileSnapshot : public AbstractPlaybackFile {
    Q_OBJECT

    enum ColumnType {
        ColumnCheck,
        ColumnChannel,
        ColumnDisk,
        ColumnTime,
        ColumnSize
    };

public:
    explicit PlaybackFileSnapshot(QWidget *parent = nullptr);

    void updateTableList() override;
    void clear() override;

    int waitForSnapshot(int winid);

    bool hasNotExportedFile() const;

    //export
    void exportSnapshot();
    void exportAllSnapshot();

    void dealMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_SET_PLAYBACK_SNAPSHOT(MessageReceive *message);
    void ON_RESPONSE_FLAG_SNAPHOST_SPLIT_PLAYBACK(MessageReceive *message);

protected:
    void onLanguageChanged();

    virtual int pageCount() override;
    virtual int itemCount() override;
    virtual void dealSelected() override;
    //table
    virtual QStringList tableHeaders() override;
    virtual void initializeTableView() override;

private:
protected slots:
    virtual void onItemClicked(int row, int column) override;

private:
    QMap<PlaybackFileKey, resp_snapshot_state> m_fileMap;

    int m_currentCount = 0;
    int m_currentIndex = 0;
    int m_snapshotSucceedCount = 0;

    QEventLoop m_eventLoop;
};

#endif // PLAYBACKFILESNAPSHOT_H
