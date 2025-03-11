#ifndef PLAYBACKFILETAG_H
#define PLAYBACKFILETAG_H

#include "AbstractPlaybackFile.h"
#include <QEventLoop>

class MsWaitting;

class PlaybackFileTag : public AbstractPlaybackFile {
    Q_OBJECT

    enum ColumnType {
        ColumnCheck,
        ColumnChannel,
        ColumnName,
        ColumnTime,
        ColumnEdit,
        ColumnDelete
    };

public:
    explicit PlaybackFileTag(QWidget *parent = nullptr);

    void updateTableList() override;
    void clear() override;

    int waitForTagAll(const QMap<int, QDateTime> &dateTimeMap, const QString &name = QString());
    int waitForTag(const QDateTime &dateTime, const QString &name = QString());

    void dealMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_TEMP_TAGS_PLAYBACK(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_TEMP_TAGS_PLAYBACK(MessageReceive *message);
    void ON_RESPONSE_FLAG_EDIT_TEMP_TAGS_PLAYBACK(MessageReceive *message);
    void ON_RESPONSE_FLAG_REMOVE_TEMP_TAGS_PLAYBACK(MessageReceive *message);
    void deleteAllTag();
    void deleteTag();

protected:
    void showEvent(QShowEvent *) override;

    void onLanguageChanged();

    virtual int pageCount() override;
    virtual int itemCount() override;
    //table
    virtual QStringList tableHeaders() override;
    virtual void initializeTableView() override;

private:
    void getTempTags(int page);

public slots:
    void getTags();

protected slots:
    virtual void onItemClicked(int row, int column) override;

    virtual void on_toolButton_firstPage_clicked() override;
    virtual void on_toolButton_previousPage_clicked() override;
    virtual void on_toolButton_nextPage_clicked() override;
    virtual void on_toolButton_lastPage_clicked() override;
    virtual void on_pushButton_go_clicked() override;

private:
    QMap<PlaybackFileKey, resp_search_tags> m_fileMap;
    QEventLoop m_eventLoop;

    int m_searchPageCount = 0;
    int m_searchPage = 0;

    int m_tagCount = 0;
    int m_tagSuccessCount = 0;
    MsWaitting *m_waitting = nullptr;
};

#endif // PLAYBACKFILETAG_H
