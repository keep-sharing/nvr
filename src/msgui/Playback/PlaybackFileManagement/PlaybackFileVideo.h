#ifndef PLAYBACKFILEVIDEO_H
#define PLAYBACKFILEVIDEO_H

#include "AbstractPlaybackFile.h"
#include <QEventLoop>

class PlaybackFileVideo : public AbstractPlaybackFile {
    Q_OBJECT

    enum ColumnType {
        ColumnCheck,
        ColumnChannel,
        ColumnTime,
        ColumnSize
    };

public:
    explicit PlaybackFileVideo(QWidget *parent = nullptr);

    void updateTableList() override;
    void clear() override;

    int waitForCutPlayback(const QDateTime &begin, const QDateTime &end);

    bool hasNotExportedFile() const;

    //export
    void exportVideo();
    void exportAllVideo();

    //
    void dealMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SEARCH_COM_PAGE(MessageReceive *message);

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

    int m_currentSid = -1;
    int m_currentCount = 0;

    QEventLoop m_eventLoop;
};

#endif // PLAYBACKFILEVIDEO_H
