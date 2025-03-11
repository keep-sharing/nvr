#ifndef ABSTRACTPLAYBACKFILE_H
#define ABSTRACTPLAYBACKFILE_H

#include "PlaybackFileKey.h"

#include <QModelIndex>
#include <QWidget>

class MessageReceive;

const int FileInfoRole = Qt::UserRole + 50;
const int FileKeyRole = Qt::UserRole + 51;

namespace Ui {
class AbstractPlaybackFile;
}

class AbstractPlaybackFile : public QWidget {
    Q_OBJECT

public:
    explicit AbstractPlaybackFile(QWidget *parent = nullptr);
    ~AbstractPlaybackFile();

    virtual void clear();
    virtual void dealMessage(MessageReceive *message) = 0;

    static bool hasFreeSid();
    static void closeAllSid();

protected:
    virtual int pageCount() = 0;
    virtual int itemCount() = 0;
    virtual void dealSelected();
    //table
    virtual QStringList tableHeaders() = 0;
    virtual void initializeTableView() = 0;
    virtual void updateTableList();

    int currentPage();

protected slots:
    virtual void onLanguageChanged();

    virtual void onTableHeaderClicked(bool checked);
    virtual void onItemClicked(int row, int column);

    virtual void on_toolButton_firstPage_clicked();
    virtual void on_toolButton_previousPage_clicked();
    virtual void on_toolButton_nextPage_clicked();
    virtual void on_toolButton_lastPage_clicked();
    virtual void on_pushButton_go_clicked();

protected:
    Ui::AbstractPlaybackFile *ui;

    int m_pageIndex = 0;
    int m_pageCount = 0;

    static QMap<int, int> s_tempSidMap;

    bool m_alreadyExport = false;
};

#endif // ABSTRACTPLAYBACKFILE_H
